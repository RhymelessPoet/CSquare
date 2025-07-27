#include "GraphicsCommandBuffer.h"
#include "GraphicsCommandBufferDescriptor.h"
#include "GraphicsCommands.h"

namespace CS
{
GraphicsCommandBuffer::GraphicsCommandBuffer(GraphicsCommandBufferDescriptor* descriptor)
    : IGraphicsResource(descriptor)
{}

GraphicsCommandBuffer::~GraphicsCommandBuffer() {}

GraphicsCommandBuffer& GraphicsCommandBuffer::BeginPass(RenderTarget renderTarget)
{
    descriptor<GraphicsCommandBufferDescriptor>().Push<Command_BeginPass>(renderTarget.GetID());
    return *this;
}

GraphicsCommandBuffer& GraphicsCommandBuffer::EndPass()
{
    descriptor<GraphicsCommandBufferDescriptor>().Push<Command_EndPass>();
    return *this;
}

GraphicsCommandBuffer& GraphicsCommandBuffer::Clear(std::optional<Color> clearColor)
{
    descriptor<GraphicsCommandBufferDescriptor>().Push<Command_Clear>(clearColor);
    return *this;
}

void GraphicsCommandBuffer::Clear()
{
    descriptor<GraphicsCommandBufferDescriptor>().Clear();
}

} // namespace CS
