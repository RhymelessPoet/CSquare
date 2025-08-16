#pragma once
#include "IGraphicsResource.h"

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

private:
    IndexBuffer(GraphicsBufferDescriptor* descriptor);
};

} // namespace CS
