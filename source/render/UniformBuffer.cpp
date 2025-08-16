#include "UniformBuffer.h"
#include "GraphicsBufferDescriptor.h"

namespace CS
{

UniformBuffer::UniformBuffer(GraphicsBufferDescriptor* descriptor) : IGraphicsResource(descriptor) {}

void UniformBuffer::UpdateData(const void* data, size_t size)
{
    if (!IsValid()) {
        return;
    }

    descriptor<GraphicsBufferDescriptor>().UpdateData(data, size);
}

} // namespace CS
