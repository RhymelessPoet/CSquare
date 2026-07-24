#include "VertexBuffer.h"
#include "GraphicsBufferDescriptor.h"

namespace CS
{
VertexBuffer::VertexBuffer(GraphicsBufferDescriptor* descriptor) : IGraphicsResource(descriptor) {}

void VertexBuffer::UpdateData(const void* data, size_t size)
{
    if (!IsValid()) {
        return;
    }

    descriptor<GraphicsBufferDescriptor>().UpdateData(data, size);
}

void VertexBuffer::UpdateData(std::span<const std::byte> data, size_t offset)
{
    if (!IsValid()) {
        return;
    }
    descriptor<GraphicsBufferDescriptor>().UpdateData(data, offset);
}

} // namespace CS
