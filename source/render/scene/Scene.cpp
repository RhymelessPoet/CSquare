#include "Scene.h"
#include "Camera.h"
#include "IRenderSystem.h"
#include "SceneObject.h"

namespace CS
{
Scene::Scene()
{
    m_root = std::make_shared<SceneObject>();
}

Scene::~Scene() {}

void Scene::OnUpdate()
{
    for (const auto& system : m_systems) {
        system->OnUpdate();
    }
}

void Scene::OnRender(RenderContext& context)
{
    for (const auto& renderSystem : m_renderSystems) {
        renderSystem->OnRender(context);
    }
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

} // namespace CS
