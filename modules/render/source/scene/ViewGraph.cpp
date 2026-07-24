#include "ViewGraph.h"
#include "Camera.h"
#include "Scene.h"
#include "SceneEvents.h"
#include "View.h"
#include "Graph.h"
#include "graphics/GraphicsAPI.h"
#include "materials/IMaterialConfiguration.h"
#include "materials/Material.h"
#include "renderer/RenderContext.h"

namespace CS
{
using namespace std;

using ClassifiedViews = std::unordered_map<EViewType, std::vector<std::shared_ptr<View>>>;

template <>
struct ImplData<ViewGraph>
{
    ImplData(std::shared_ptr<GraphicsAPI> graphicsAPI) : graphicsAPI(std::move(graphicsAPI)) {}

    void sort();

    std::shared_ptr<GraphicsAPI> graphicsAPI;
    DAG<std::shared_ptr<View>, GenerationID<ViewID>> viewGraph;
    std::shared_ptr<View> mainView;
    std::vector<std::shared_ptr<View>> views;
    std::map<std::shared_ptr<Scene>, ClassifiedViews> sceneToViews;
};

static void Connect(GraphicsAPI& graphicsAPI, std::span<const SlotDescription> connects, const View& src, View& dst)
{
    auto renderTarget = src.GetRenderTarget();
    for (const auto& slot : connects) {
        if (!slot.connectedSlot.has_value()) {
            continue;
        }
        auto targetSlot = slot.connectedSlot.value();
        if (slot.type == SlotType::Color) {
            dst.SetInput(targetSlot, graphicsAPI.GetColorAttachment(renderTarget));
        }
        if (slot.type == SlotType::Depth) {
            dst.SetInput(targetSlot, graphicsAPI.GetDepthAttachment(renderTarget));
        }
        if (slot.type == SlotType::DepthStencil) {
            dst.SetInput(targetSlot, graphicsAPI.GetDepthStencilAttachment(renderTarget));
        }
    }
}

ViewGraph::ViewGraph(std::shared_ptr<GraphicsAPI> graphicsAPI) : ImplBase(std::move(graphicsAPI))
{
    impl().mainView = CreateView(EViewType::Make<"3D_Main">());
}

std::shared_ptr<View> ViewGraph::GetMainView() const
{
    return impl().mainView;
}

void ViewGraph::Recompute()
{
    impl().sort();
}

std::shared_ptr<View> ViewGraph::CreateView(EViewType type)
{
    auto id = impl().viewGraph.AddVertex();
    auto view = View::create((ViewID)id, type);
    impl().viewGraph.SetVertexData(id, view);
    return view;
}

std::shared_ptr<View> ViewGraph::CreateView(EViewType type, RenderTarget target)
{
    auto id = impl().viewGraph.AddVertex();
    auto view = View::create((ViewID)id, type, target);
    impl().viewGraph.SetVertexData(id, view);
    return view;
}

std::unique_ptr<IEvent> ViewGraph::OnEvent(std::unique_ptr<IEvent> event)
{
    if (auto event_ = dynamic_cast<NewMaterialInScene*>(event.get()); event_ != nullptr) {
        auto material = event_->GetMaterial();
        auto scene = event_->GetScene();
        onNewMaterialInScene(material, scene);
        return nullptr;
    }
    return nullptr;
}

void ViewGraph::OnRender(RenderContext& context) const
{
    // Expose the main view's camera so pre-passes (shadow map, etc.) can cull against
    // what the user will actually see.
    if (impl().mainView) {
        context.SetObserverCamera(impl().mainView->GetCamera());
    }
    for (const auto& view : impl().views) {
        view->OnRender(context);
    }
}

void ViewGraph::onNewMaterialInScene(const std::shared_ptr<Material>& material, const std::shared_ptr<Scene>& scene)
{
    std::shared_ptr<View> mainView;
    auto& config = material->GetConfiguration();

    if (config.GetViewType() == EViewType::Make<"3D_Main">()) {
        mainView = impl().mainView;
        for (const auto& requisiteMaterial : material->GetRequisiteMaterials()) {
            auto predecessorView = rearrangePredecessorView(requisiteMaterial, scene);
            auto slots = requisiteMaterial->GetConfiguration().GetTargetSlots();
            Connect(*impl().graphicsAPI, slots, *predecessorView, *mainView);
            impl().viewGraph.AddEdge(mainView->GetID(), predecessorView->GetID());
        }
    } else {
        mainView = rearrangePredecessorView(material, scene);
    }
    Recompute();
}

void ViewGraph::onNewLightInScene(const std::shared_ptr<SceneObject>& sceneObject, const std::shared_ptr<Scene>& scene)
{
    auto& classifiedViews = impl().sceneToViews[scene];
    if (auto itr = classifiedViews.find(EViewType::Make<"Shadow_Map">()); itr != classifiedViews.end())
    { // view created but not bind light
        for (auto& view : itr->second) {
            configShadowView(view); // view bind light
        }
    }
}

std::shared_ptr<View> ViewGraph::rearrangePredecessorView(const std::shared_ptr<Material>& material,
                                                          const std::shared_ptr<Scene>& scene)
{
    const auto& config = material->GetConfiguration();
    auto viewType = config.GetViewType();
    auto views = getViews(scene, config);
    if (views.empty()) {
        return nullptr;
    }
    auto view = views[0]; // Assume only one view

    for (const auto& requisiteMaterial : material->GetRequisiteMaterials()) {
        auto predecessorView = rearrangePredecessorView(requisiteMaterial, scene);
        auto slots = requisiteMaterial->GetConfiguration().GetTargetSlots();
        Connect(*impl().graphicsAPI, slots, *predecessorView, *view);
        impl().viewGraph.AddEdge(view->GetID(), predecessorView->GetID());
    }

    return view;
}

std::span<const std::shared_ptr<View>> ViewGraph::getViews(std::shared_ptr<Scene> scene,
                                                           const IMaterialConfiguration& config)
{
    auto viewType = config.GetViewType();

    for (const auto& [type, views] : impl().sceneToViews[scene]) {
        if (type == viewType && !views.empty()) { // assume the first view is the most suitable one for reuse
            return std::span<const std::shared_ptr<View>>(views.begin(), views.end());
        }
    }
    auto targetSize = config.GetTargetSize().value_or(Size2u{1280u, 720u});
    auto renderTarget = impl().graphicsAPI->CreateRenderTarget(targetSize);

    for (const auto& slot : config.GetTargetSlots()) {
        if (slot.type == SlotType::Color) {
            renderTarget.SetColorAttachment(impl().graphicsAPI->CreateTexture(TextureFormat::RGBA8Unorm, targetSize));
        } else if (slot.type == SlotType::Depth) {
            renderTarget.SetDepthAttachment(impl().graphicsAPI->CreateTexture(TextureFormat::Depth32, targetSize));
        } else if (slot.type == SlotType::Stencil) {
            renderTarget.SetDepthStencilAttachment(
                impl().graphicsAPI->CreateTexture(TextureFormat::Stencil32, targetSize));
        } else if (slot.type == SlotType::DepthStencil) {
            renderTarget.SetDepthStencilAttachment(
                impl().graphicsAPI->CreateTexture(TextureFormat::Depth24Stencil8, targetSize));
        }
    }

    auto view = CreateView(viewType, renderTarget);
    view->SetScene(scene);

    if (viewType == EViewType::Make<"Shadow_Map">()) {
        configShadowView(view);
    }

    auto& views = impl().sceneToViews[scene][viewType];
    views.push_back(view);

    return std::span<const std::shared_ptr<View>>(views.begin(), views.end());
}

void ViewGraph::configShadowView(std::shared_ptr<View> view)
{
    const auto& scene = view->GetScene();
    auto lights = scene->GetLights(ELightType::Make<"Directional">());
    if (lights.empty()) {
        return;
    }
    view->SetCamera(std::make_shared<Camera>(lights[0]));
}

void ImplData<ViewGraph>::sort()
{
    auto vertices = viewGraph.GetTopologicallySortedVertices();
    views.clear();
    for (const auto& vertexID : vertices) {
        auto result = viewGraph.GetVertex(vertexID).and_then([&](const auto* vertex) {
            views.push_back(vertex->data);
            return std::expected<void, bool>();
        });
        if (!result.has_value()) {
            // std::cerr << "Error: Vertex with ID " << vertexID << " not found in view graph.\n";
        }
    }
}

} // namespace CS
