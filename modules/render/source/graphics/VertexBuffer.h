#pragma once
#include "IGraphicsResource.h"
#include <span>
namespace CS
{
class GraphicsBufferDescriptor;

class VertexBuffer final : public IGraphicsResource
{
public:
    friend class GraphicsAPI;
    VertexBuffer() = default;
    ~VertexBuffer() = default;

    void UpdateData(const void* data, size_t size);
    void UpdateData(std::span<const std::byte> data, size_t offset = 0);

private:
    VertexBuffer(GraphicsBufferDescriptor* descriptor);
};

} // namespace CS
