#pragma once
#include "IGraphicsResource.h"

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

private:
    VertexBuffer(GraphicsBufferDescriptor* descriptor);
};

} // namespace CS
