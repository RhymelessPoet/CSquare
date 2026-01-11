#include "Scene.h"
#include "Camera.h"
#include "IRenderSystem.h"
#include "MeshRenderer.h"
#include "SceneObject.h"
#include "geometry/GeometryNode.h"
#include "materials/Material.h"
#include "renderer/MaterialCompiler.h"
#include "renderer/RenderContext.h"

namespace CS
{
Scene::Scene(std::shared_ptr<SceneObjectComposer> composer) : m_composer(std::move(composer))
{
    m_root = std::make_shared<SceneObject>("Root");
}

Scene::~Scene() {}

void Scene::OnRender(RenderContext& context)
{
    collectRenderables(m_root);
    for (auto component : m_components) {
        auto meshRender = dynamic_cast<MeshRenderer*>(component);
        if (meshRender != nullptr) {
            meshRender->OnUpdate();

            auto material = meshRender->GetGeometryNode(0u)->GetMaterial()->GetMaterial();
            m_materials[material->GetID()] = material;
        }
    }

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

    for (const auto component : m_components) {
        component->OnRender(context);
    }
    m_components.clear();
}

std::shared_ptr<Camera> Scene::CreateCamera()
{
    return std::make_shared<Camera>(shared_from_this());
}

std::shared_ptr<SceneObject> Scene::CreateSceneObject(std::shared_ptr<SceneObject> parent)
{
    auto sceneObject = std::make_shared<SceneObject>();
    if (parent != nullptr) {
        sceneObject->SetParent(parent);
    } else {
        sceneObject->SetParent(m_root);
    }
    return sceneObject;
}

void Scene::collectRenderables(std::shared_ptr<SceneObject> object)
{
    if (auto meshRenderer = object->GetComponent<MeshRenderer>()) {
        m_components.push_back(meshRenderer);
    }

    for (const auto& child : object->GetChildren()) {
        collectRenderables(child);
    }
}

} // namespace CS
