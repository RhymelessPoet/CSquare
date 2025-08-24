#include "CameraSystem.h"
#include "CameraComponent.h"

namespace CS
{
void CameraSystem::OnUpdate()
{
    for (auto component : m_components) {
        component->OnUpdate();
    }
}

} // namespace CS
