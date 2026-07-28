#pragma once
#include "IGraphicsResource.h"
#include <span>
#include <vector>

namespace CS
{
class GraphicsBufferDescriptor;
class GraphicsAPI;

class StorageBuffer final : public IGraphicsResource
{
public:
    friend class GraphicsAPI;
    StorageBuffer() = default;
    ~StorageBuffer() = default;

    void UpdateData(const void* data, size_t size);
    void UpdateData(std::span<const std::byte> data, size_t offset = 0);
    size_t GetSize() const;

private:
    explicit StorageBuffer(GraphicsBufferDescriptor* descriptor);
};
} // namespace CS
