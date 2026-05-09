#pragma once
#include "IGraphicsResource.h"
#include <span>

namespace CS
{
class GraphicsBufferDescriptor;

class UniformBuffer final : public IGraphicsResource
{
public:
    friend class GraphicsAPI;
    UniformBuffer(/* args */) = default;
    ~UniformBuffer() = default;

    void UpdateData(const void* data, size_t size);
    // Partial upload: uploads `data` starting at byte `offset` in the GPU buffer (glBufferSubData).
    void UpdateData(std::span<const std::byte> data, size_t offset);

private:
    UniformBuffer(GraphicsBufferDescriptor* descriptor);
};

} // namespace CS
