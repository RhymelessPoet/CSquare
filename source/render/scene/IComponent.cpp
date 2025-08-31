#include "IComponent.h"
#include "ISystem.h"
#include "SceneObject.h"

namespace CS
{
IComponent::IComponent(std::shared_ptr<SceneObject> owner) : m_owner(owner) {}

IComponent::~IComponent()
{
    if (m_system != nullptr) {
        m_system->removeComponent(this);
    }
}

} // namespace CS
