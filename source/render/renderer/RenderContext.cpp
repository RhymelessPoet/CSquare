#include "RenderContext.h"
#include "graphics/GraphicsAPI.h"

namespace CS
{
RenderContext::RenderContext(std::shared_ptr<GraphicsAPI> graphicsAPI)
    : m_graphicsAPI(std::move(graphicsAPI)), m_commandBuffer(m_graphicsAPI->CreateCommandBuffer())
{}

} // namespace CS
