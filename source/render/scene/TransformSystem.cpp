#include "SystemContext.h"
#include "Transform.h"
#include "TransformSystem.h"

namespace CS
{
void TransformSystem::OnUpdate(SystemContext& context)
{
    for (auto component : m_components) {
        component->OnUpdate(context);
    }
}

} // namespace CS
