#include "Scene.h"
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
    for (const auto& renderSystem : m_renderSystems) {
        renderSystem->OnUpdate();
    }
}

void Scene::OnRender(RenderContext& context)
{
    for (const auto& renderSystem : m_renderSystems) {
        renderSystem->OnRender(context);
    }
}

} // namespace CS
