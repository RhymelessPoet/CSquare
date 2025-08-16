#pragma once
#include "IGraphicsResource.h"

namespace CS
{
class GraphicsBufferDescriptor;

class UniformBuffer final : public IGraphicsResource
{
public:
    UniformBuffer(/* args */) = default;
    ~UniformBuffer() = default;

    void UpdateData(const void* data, size_t size);

private:
    UniformBuffer(GraphicsBufferDescriptor* descriptor);
};

} // namespace CS
