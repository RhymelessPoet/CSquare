#include "Scene.h"
#include "Camera.h"
#include "IRenderSystem.h"
#include "MeshRenderer.h"
#include "SceneEvents.h"
#include "SceneObject.h"
#include "SceneObjectEvents.h"
#include "geometry/GeometryNode.h"
#include "materials/Material.h"
#include "renderer/MaterialCompiler.h"
#include "renderer/RenderContext.h"

namespace CS
{
Scene::Scene(std::shared_ptr<SceneObjectComposer> composer, std::string_view name)
    : m_composer(std::move(composer)), m_name(name)
{
    m_root = std::make_shared<SceneObject>("Root");
}

Scene::~Scene() {}

void Scene::OnRender(RenderContext& context)
{
    collectRenderables(m_root);
    // for (auto renderable : m_renderables) {
    //     auto meshRender = dynamic_cast<MeshRenderer*>(renderable);
    //     if (meshRender == nullptr) {
    //         continue;
    //     }
    //     if (auto go = meshRender->GetGeometryNode(0u); go != nullptr) {
    //         auto material = go->GetMaterial()->GetMaterial();
    //         m_materials[material->GetID()] = material;
    //     }
    // }

    auto camera = context.GetCamera();

    const auto& cameraPos = camera->GetPosition();
    const auto viewMatrix = camera->GetViewMatrix().Transposed();
    const auto projectionMatrix = camera->GetProjectionMatrix().Transposed();
    for (auto& [_, material] : m_materials) {
        auto noError = material->SetUniformValue(std::string_view("projection"), projectionMatrix.ToStdVector());
        noError = noError && material->SetUniformValue(std::string_view("view"), viewMatrix.ToStdVector());
        noError = noError && material->SetUniformValue(std::string_view("camera_position"), cameraPos);
        noError = noError && material->SetUniformValue(std::string_view("light_direction"),
                                                       Vector3f{7.0f, 3.0f, 1.0f}.Normalized());
        noError = noError && material->SetUniformValue(std::string_view("light_color"), Vector3f{1.0f, 1.0f, 1.0f});
        noError = noError && material->SetUniformValue(std::string_view("light_intensity"), 10.0f);

        auto& materialCompiler = context.GetMaterialCompiler();
        material->GetDefaultInstance().Apply(materialCompiler);

        auto shaderBindingSet = materialCompiler.GetShaderBindingSet(material->GetDefaultInstance());
        context.GetCommandBuffer().Bind(shaderBindingSet);
    }

    for (const auto renderable : m_renderables) {
        renderable->OnRender(context);
    }
    m_renderables.clear();
}

std::shared_ptr<Camera> Scene::CreateCamera()
{
    return std::make_shared<Camera>(shared_from_this());
}

std::shared_ptr<SceneObject> Scene::CreateSceneObject(std::shared_ptr<SceneObject> parent)
{
    auto sceneObject = std::make_shared<SceneObject>();
    sceneObject->SetScene(shared_from_this());
    if (parent != nullptr) {
        sceneObject->SetParent(parent);
    } else {
        sceneObject->SetParent(m_root);
    }
    return sceneObject;
}

const AABB& Scene::GetAABB(bool reCompute)
{
    if (reCompute) {
        traverseWith(m_root, [this](std::shared_ptr<SceneObject> object) {
            if (auto meshRenderer = object->GetComponent<MeshRenderer>()) {
                auto obb = meshRenderer->GetWorldBoundingBox();
                m_box.Include(obb);
            }
        });
    }
    return std::as_const(*this).GetAABB();
}

std::unique_ptr<IEvent> Scene::OnEvent(std::unique_ptr<IEvent> event)
{
    if (auto event_ = dynamic_cast<NewGeometryNode*>(event.get()); event_ != nullptr) {
        return onEvent(event_);
    }
    return event;
}

void Scene::traverseWith(std::shared_ptr<SceneObject> object,
                         const std::function<void(std::shared_ptr<SceneObject>)>& func)
{
    func(object);
    for (const auto& child : object->GetChildren()) {
        if (!child->IsActive()) {
            continue;
        }
        traverseWith(child, func);
    }
}

void Scene::collectRenderables(std::shared_ptr<SceneObject> object)
{
    if (auto meshRenderer = object->GetComponent<MeshRenderer>()) {
        m_renderables.push_back(meshRenderer);
    }

    for (const auto& child : object->GetChildren()) {
        if (!child->IsActive()) {
            continue;
        }
        collectRenderables(child);
    }
}

std::unique_ptr<IEvent> Scene::onEvent(NewGeometryNode* event)
{
    auto materialInstance = event->GetGeometryNode()->GetMaterial();
    auto result = m_materials.emplace(materialInstance->GetMaterial()->GetID(), materialInstance->GetMaterial());
    if (result.second) {
        return std::make_unique<NewMaterialInScene>(shared_from_this(), result.first->second);
    }
    return std::unique_ptr<IEvent>();
}

} // namespace CS
