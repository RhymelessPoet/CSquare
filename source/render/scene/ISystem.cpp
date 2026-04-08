#include "ISystem.h"
#include "IComponent.h"
#include "SceneObject.h"

namespace CS
{

ISystem::ISystem(/* args */) {}

ISystem::~ISystem() {}

bool ISystem::assembleComponent(std::shared_ptr<SceneObject> so, std::unique_ptr<IComponent> component)
{
    component->m_system = this;
    m_components.push_back(component.get());
    return so->AddComponent(std::move(component));
}

void ISystem::removeComponent(IComponent* component)
{
    auto it = std::remove(m_components.begin(), m_components.end(), component);
    if (it != m_components.end()) {
        m_components.erase(it, m_components.end());
    }
}

std::unique_ptr<IEvent> ISystem::dispatch(IEventDispatcher* nextDispatcher, std::unique_ptr<IEvent> event)
{
    nextDispatcher->PushEvent(std::move(event));
    return nullptr;
}

InvalidSystem& InvalidSystem::Instance()
{
    static InvalidSystem instance;
    return instance;
};

} // namespace CS
