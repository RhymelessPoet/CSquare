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

GraphicsCommandBuffer& GraphicsCommandBuffer::SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
{
    descriptor<GraphicsCommandBufferDescriptor>().Push<Command_SetViewport>(x, y, width, height);
    return *this;
}

GraphicsCommandBuffer& GraphicsCommandBuffer::Bind(GraphicsPipeline pipeline)
{
    descriptor<GraphicsCommandBufferDescriptor>().Push<Command_BindPipeline>(pipeline.GetID());
    return *this;
}

GraphicsCommandBuffer& GraphicsCommandBuffer::Bind(GraphicsInputAssembly inputAssembly)
{
    descriptor<GraphicsCommandBufferDescriptor>().Push<Command_BindInputAssembly>(inputAssembly.GetID());
    return *this;
}

GraphicsCommandBuffer& GraphicsCommandBuffer::DrawIndexed(uint32_t count, uint32_t indexOffset)
{
    descriptor<GraphicsCommandBufferDescriptor>().Push<Command_DrawIndexed>(count, indexOffset);
    return *this;
}

void GraphicsCommandBuffer::ClearBuffer()
{
    descriptor<GraphicsCommandBufferDescriptor>().Clear();
}

} // namespace CS
