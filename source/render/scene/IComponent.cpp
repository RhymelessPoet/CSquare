#include "IComponent.h"
#include "ISystem.h"
#include "SceneObject.h"

namespace CS
{
IComponent::IComponent(std::shared_ptr<SceneObject> owner) : m_owner(owner)
{
    setState(EComponentState::Make<"New">(), true);
}

IComponent::~IComponent()
{
    if (m_system != nullptr) {
        m_system->removeComponent(this);
    }
}

void IComponent::setState(EComponentState state, bool value)
{
    m_stateFlags.set(static_cast<underlying_type_t<EComponentState>>(state), value);
}

bool IComponent::isOn(EComponentState state) const
{
    return m_stateFlags.test(static_cast<underlying_type_t<EComponentState>>(state));
}

} // namespace CS
