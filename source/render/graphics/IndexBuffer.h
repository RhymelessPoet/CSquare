#pragma once
#include "IGraphicsResource.h"
#include <span>

namespace CS
{
class GraphicsBufferDescriptor;

class IndexBuffer final : public IGraphicsResource
{
public:
    friend class GraphicsAPI;
    IndexBuffer() = default;
    ~IndexBuffer() = default;

    void UpdateData(const void* data, size_t size);
    void UpdateData(std::span<const std::byte> data, size_t offset = 0);

private:
    IndexBuffer(GraphicsBufferDescriptor* descriptor);
};

} // namespace CS
