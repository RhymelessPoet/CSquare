#include "GraphicsCommandBufferDescriptor.h"

namespace CS
{

GraphicsCommandBufferDescriptor::GraphicsCommandBufferDescriptor(size_t id, std::shared_ptr<GraphicsGLImpl> graphicsAPI)
    : IGraphicsResourceDescriptor(id, graphicsAPI)
{}

} // namespace CS
