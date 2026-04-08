#include "SystemContext.h"
#include "core/event/IEventDispatcher.h"

namespace CS
{
SystemContext::SystemContext(RenderModuleContext& moduleContext)
    : m_moduleContext(moduleContext), m_eventDispatcher(InvalidEventDispatcher::Instance())
{}

} // namespace CS
