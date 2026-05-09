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
    if (value) {
        m_stateFlags.Set(state);
    } else {
        m_stateFlags.Reset(state);
    }
}

bool IComponent::IsOn(EComponentState state) const
{
    return m_stateFlags.Test(state);
}

} // namespace CS
