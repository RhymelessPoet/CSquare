#include "MeshRenderSystem.h"
#include "MeshRenderer.h"

namespace CS
{
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
