#include "IRenderSystem.h"

namespace CS
{
IRenderSystem::IRenderSystem(std::shared_ptr<GraphicsAPI> graphicsAPI) : m_graphicsAPI(std::move(graphicsAPI)) {}

} // namespace CS
