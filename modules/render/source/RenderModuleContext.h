#pragma once
#include <type_traits>

namespace CS
{
class IEventDispatcher;

class RenderModuleContext
{
public:
    RenderModuleContext(IEventDispatcher* dispatcher);

    IEventDispatcher* GetEventDispatcher() { return m_eventDispatcher; }

private:
    IEventDispatcher* m_eventDispatcher;
};

} // namespace CS
