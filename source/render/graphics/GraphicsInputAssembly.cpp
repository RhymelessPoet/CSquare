#include "GraphicsInputAssembly.h"
#include "GraphicsInputAssemblyDescriptor.h"

namespace CS
{
GraphicsInputAssembly::GraphicsInputAssembly(GraphicsInputAssemblyDescriptor* descriptor)
    : IGraphicsResource(descriptor)
{}

void GraphicsInputAssembly::SetVertexInputLayout(std::shared_ptr<VertexInputLayout> inputLayout)
{
    descriptor<GraphicsInputAssemblyDescriptor>().SetVertexInputLayout(std::move(inputLayout));
}

void GraphicsInputAssembly::SetVertexInput(uint32_t binding, VertexBuffer buffer, uint32_t offset)
{
    descriptor<GraphicsInputAssemblyDescriptor>().SetVertexInput(binding, buffer.GetID(), offset);
}

void GraphicsInputAssembly::SetIndexBuffer(IndexBuffer indexBuffer, uint16_t)
{
    descriptor<GraphicsInputAssemblyDescriptor>().SetIndexBuffer(indexBuffer.GetID(), IndexFormat::UInt16);
}

void GraphicsInputAssembly::SetIndexBuffer(IndexBuffer indexBuffer, uint32_t)
{
    descriptor<GraphicsInputAssemblyDescriptor>().SetIndexBuffer(indexBuffer.GetID(), IndexFormat::UInt32);
}
} // namespace CS
