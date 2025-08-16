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

} // namespace CS
