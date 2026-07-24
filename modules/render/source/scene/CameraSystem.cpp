#include "CameraSystem.h"
#include "CameraComponent.h"
#include "SystemContext.h"

namespace CS
{
void CameraSystem::OnUpdate(SystemContext& context)
{
    for (auto component : m_components) {
        component->OnUpdate(context);
    }
}

} // namespace CS
