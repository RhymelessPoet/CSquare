#pragma once
#include "GraphicsCommandBuffer.h"
#include <memory>

namespace CS
{
class GraphicsAPI;

class RenderContext
{
public:
    RenderContext(std::shared_ptr<GraphicsAPI> graphicsAPI);
    virtual ~RenderContext() = default;

    GraphicsCommandBuffer GetCommandBuffer() const { return m_commandBuffer; }

private:
    std::shared_ptr<GraphicsAPI> m_graphicsAPI;
    GraphicsCommandBuffer m_commandBuffer;
};

} // namespace CS
