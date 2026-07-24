#include "SystemGraph.h"
#include "ISystem.h"
#include "RenderModuleContext.h"
#include "SceneEvents.h"
#include "SystemContext.h"

namespace CS
{

template <>
struct ImplData<SystemGraph>
{
    std::vector<std::unique_ptr<ISystem>> systems;
};

SystemGraph::SystemGraph() : PImpl<SystemGraph>() {}

void SystemGraph::OnUpdate(RenderModuleContext& context)
{
    SystemContext sysContext(context);
    for (const auto& system : systems()) {
        sysContext.SetEventDispatcher(system.get());
        system->OnUpdate(sysContext);
        system->Dispatch(this);
    }
    Dispatch(context.GetEventDispatcher());
}

std::vector<IEventListener*> SystemGraph::sift(IEvent* event) const
{
    return std::vector<IEventListener*>();
}

std::unique_ptr<IEvent> SystemGraph::dispatch(IEventDispatcher* nextDispatcher, std::unique_ptr<IEvent> event)
{
    if (auto event_ = dynamic_cast<NewMaterialInScene*>(event.get()); event_ != nullptr) {
        nextDispatcher->PushEvent(std::move(event));
        return nullptr;
    }
    return event;
}

void SystemGraph::sort() {}

std::vector<std::unique_ptr<ISystem>>& SystemGraph::systems()
{
    return impl().systems;
}

} // namespace CS
