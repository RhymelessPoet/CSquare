#include "StorageBuffer.h"
#include "GraphicsBufferDescriptor.h"

namespace CS
{
StorageBuffer::StorageBuffer(GraphicsBufferDescriptor* descriptor) : IGraphicsResource(descriptor) {}
void StorageBuffer::UpdateData(const void* data, size_t size)
{
    if (IsValid()) descriptor<GraphicsBufferDescriptor>().UpdateData(data, size);
}
void StorageBuffer::UpdateData(std::span<const std::byte> data, size_t offset)
{
    if (IsValid()) descriptor<GraphicsBufferDescriptor>().UpdateData(data, offset);
}
size_t StorageBuffer::GetSize() const
{
    return IsValid() ? descriptor<GraphicsBufferDescriptor>().GetSize() : 0u;
}
} // namespace CS
