#include "SystemContext.h"
#include "event/IEventDispatcher.h"

namespace CS
{
SystemContext::SystemContext(RenderModuleContext& moduleContext)
    : m_moduleContext(moduleContext), m_eventDispatcher(InvalidEventDispatcher::Instance())
{}

} // namespace CS
