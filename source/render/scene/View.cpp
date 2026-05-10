#include "View.h"
#include "Camera.h"
#include "Scene.h"
#include "graphics/GraphicsCommandBuffer.h"
#include "graphics/RenderTarget.h"
#include "renderer/MaterialCompiler.h"
#include "renderer/RenderContext.h"
#include <map>
#include <optional>

namespace CS
{
template <>
struct ImplData<View>
{
    ImplData(ViewID id, EViewType type) : id(id), type(type) {}
    ImplData(ViewID id, EViewType type, RenderTarget target) : id(id), renderTarget(target), type(type) {}

    std::map<uint32_t, Texture> inputs;
    std::optional<RenderTarget> renderTarget;
    std::shared_ptr<Scene> scene;
    std::shared_ptr<Camera> camera;
    ViewID id;
    EViewType type;
    Color clearColor{61.0f / 255.0f, 61.0f / 255.0f, 61.0f / 255.0f, 1.0f};
};

View::View(ConstructorTag, ViewID id, EViewType type) : PImpl<View>(id, type) {}

View::View(ConstructorTag, ViewID id, EViewType type, RenderTarget target) : PImpl<View>(id, type, target) {}

View::~View() = default;

void View::SetRenderTarget(RenderTarget target)
{
    impl().renderTarget = target;
}

RenderTarget View::GetRenderTarget() const
{
    return impl().renderTarget.value();
}

void View::SetScene(std::shared_ptr<Scene> scene)
{
    impl().scene = std::move(scene);
    if (impl().type == EViewType::Make<"3D_Main">()) {
        impl().camera = impl().scene->CreateCamera();
    }
}

std::shared_ptr<Scene> View::GetScene() const
{
    return impl().scene;
}

std::shared_ptr<Camera> View::GetCamera() const
{
    return impl().camera;
}

void View::OnRender(RenderContext& context)
{
    context.SetCamera(impl().camera);
    context.SetTargetViewType(impl().type);

    auto rtSize = GetRenderTarget().GetSize();
    context.SetViewport({0, 0, static_cast<int32_t>(rtSize.width), static_cast<int32_t>(rtSize.height)});
    context.GetMaterialCompiler().SetInputs(impl().inputs | std::views::all);

    auto cmdBuf = context.GetCommandBuffer();
    cmdBuf.BeginPass(GetRenderTarget()).Clear(impl().clearColor, 1.0f).SetViewport(0, 0, rtSize.width, rtSize.height);
    impl().scene->OnRender(context);
    cmdBuf.EndPass();
}

ViewID View::GetID() const
{
    return impl().id;
}

EViewType View::GetViewType() const
{
    return impl().type;
}

void View::SetCamera(std::shared_ptr<Camera> camera)
{
    impl().camera = std::move(camera);
}

void View::SetInput(uint32_t slotID, Texture texture)
{
    impl().inputs.emplace(slotID, texture);
}

void View::SetClearColor(const Color& color)
{
    impl().clearColor = color;
}

Color View::GetClearColor() const
{
    return impl().clearColor;
}

} // namespace CS
