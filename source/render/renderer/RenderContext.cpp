#include "RenderContext.h"
#include "graphics/GraphicsAPI.h"
#include "materials/MaterialComputer.h"

namespace CS
{
RenderContext::RenderContext(std::shared_ptr<GraphicsAPI> graphicsAPI)
    : m_graphicsAPI(std::move(graphicsAPI)), m_commandBuffer(m_graphicsAPI->CreateCommandBuffer())
{
    m_materialComputer = std::make_unique<MaterialComputer>(m_graphicsAPI);
}

} // namespace CS
