#pragma once
#include <memory>
#include <type_traits>

namespace CS
{
class IEventDispatcher;
class RenderModuleContext;

class SystemContext
{
public:
    SystemContext(RenderModuleContext& moduleContext);

    void SetEventDispatcher(IEventDispatcher* dispatcher) { m_eventDispatcher = dispatcher; }
    IEventDispatcher* GetEventDispatcher() { return m_eventDispatcher; }

private:
    std::reference_wrapper<RenderModuleContext> m_moduleContext;
    IEventDispatcher* m_eventDispatcher;
};

} // namespace CS
