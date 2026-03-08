#include "IndexBuffer.h"
#include "GraphicsBufferDescriptor.h"

namespace CS
{
IndexBuffer::IndexBuffer(GraphicsBufferDescriptor* descriptor) : IGraphicsResource(descriptor) {}

void IndexBuffer::UpdateData(const void* data, size_t size)
{
    if (!IsValid()) {
        return;
    }

    descriptor<GraphicsBufferDescriptor>().UpdateData(data, size);
}

void IndexBuffer::UpdateData(std::span<const std::byte> data, size_t offset)
{
    if (!IsValid()) {
        return;
    }
    descriptor<GraphicsBufferDescriptor>().UpdateData(data, offset);
}

} // namespace CS
