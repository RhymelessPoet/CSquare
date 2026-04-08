#include "RenderModuleContext.h"
#include "core/event/IEventDispatcher.h"

namespace CS
{
RenderModuleContext::RenderModuleContext(IEventDispatcher* dispatcher) : m_eventDispatcher(dispatcher) {}

} // namespace CS
