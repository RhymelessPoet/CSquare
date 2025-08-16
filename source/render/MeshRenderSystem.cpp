#include "MeshRenderSystem.h"
#include "MeshRenderer.h"

namespace CS
{
MeshRenderSystem::MeshRenderSystem(std::shared_ptr<GraphicsAPI> graphicsAPI) : IRenderSystem(std::move(graphicsAPI)) {}

MeshRenderSystem::~MeshRenderSystem() {}

void MeshRenderSystem::OnUpdate()
{
    for (auto& component : m_components) {
        if (component != nullptr) {
            component->OnUpdate();
        }
    }
}

void MeshRenderSystem::OnRender(RenderContext& context)
{
    for (auto& component : m_components) {
        if (auto meshRenderer = dynamic_cast<MeshRenderer*>(component)) {
            meshRenderer->OnRender(context);
        }
    }
}

} // namespace CS
