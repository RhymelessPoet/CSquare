#include "Transform.h"
#include "TransformSystem.h"


namespace CS
{
void TransformSystem::OnUpdate()
{
    for (auto component : m_components) {
        component->OnUpdate();
    }
}

} // namespace CS
