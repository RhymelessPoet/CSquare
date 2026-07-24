#include "MeshRenderSystem.h"
#include "Camera.h"
#include "MeshRenderer.h"
#include "Scene.h"
#include "SceneEvents.h"
#include "SceneObject.h"
#include "SceneObjectEvents.h"
#include "geometry/GeometryNode.h"
#include "materials/Material.h"
#include "renderer/MaterialCompiler.h"
#include "renderer/RenderContext.h"

namespace CS
{
void MeshRenderSystem::OnUpdate(SystemContext& context)
{
    for (auto component : m_components) {
        auto meshRender = dynamic_cast<MeshRenderer*>(component);
        if (meshRender == nullptr) {
            continue;
        }
        meshRender->OnUpdate(context);
    }
}

std::unique_ptr<IEvent> MeshRenderSystem::dispatch(IEventDispatcher* nextDispatcher, std::unique_ptr<IEvent> event)
{
    if (auto event_ = dynamic_cast<NewMaterialInScene*>(event.get()); event_ != nullptr) {
        nextDispatcher->PushEvent(std::move(event));
        return nullptr;
    }
    return event;
}

} // namespace CS
