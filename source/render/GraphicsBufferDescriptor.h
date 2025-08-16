#pragma once

#include "IGraphicsResourceDescriptor.h"

namespace CS
{

class GraphicsBufferDescriptor final : public IGraphicsResourceDescriptor
{
public:
    enum class BufferType
    {
        VertexBuffer,
        IndexBuffer,
        UniformBuffer,
        StorageBuffer,
        IndirectBuffer
    };
    GraphicsBufferDescriptor(size_t id, std::shared_ptr<GraphicsGLImpl> graphicsAPI);

    virtual bool IsBuild() const override;
    virtual void Destroy() override;

    virtual bool IsDirty() const override;

    void SetBufferType(BufferType type) { m_bufferType = type; }
    BufferType GetBufferType() const { return m_bufferType; }
    void SetNativeBuffer(uint32_t buffer);
    uint32_t GetNativeBuffer() const;

    bool UpdateData(const void* data, size_t size);

    void SetSize(size_t size) { m_size = size; }
    size_t GetSize() const { return m_size; }

protected:
    virtual bool build() override;

private:
    BufferType m_bufferType{BufferType::VertexBuffer};
    uint32_t m_bufferID{0u};
    size_t m_size{0u};
};

} // namespace CS