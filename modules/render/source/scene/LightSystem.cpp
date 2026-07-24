#include "LightSystem.h"
#include "LightComponent.h"
#include "SceneEvents.h"

namespace CS
{
void LightSystem::OnUpdate(SystemContext& context)
{
    for (auto component : m_components) {
        component->OnUpdate(context);
    }
}

std::unique_ptr<IEvent> LightSystem::dispatch(IEventDispatcher* nextDispatcher, std::unique_ptr<IEvent> event)
{
    if (auto event_ = dynamic_cast<NewLightInScene*>(event.get()); event_ != nullptr) {
        nextDispatcher->PushEvent(std::move(event));
        return nullptr;
    }
    return event;
}

} // namespace CS
