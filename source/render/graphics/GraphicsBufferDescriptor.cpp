#include "GraphicsBufferDescriptor.h"
#include "graphics/GraphicsGLImpl.h"

namespace CS
{
GraphicsBufferDescriptor::GraphicsBufferDescriptor(size_t id, std::shared_ptr<GraphicsGLImpl> graphicsAPI)
    : IGraphicsResourceDescriptor(id, std::move(graphicsAPI))
{}

bool GraphicsBufferDescriptor::IsBuild() const
{
    return m_bufferID != 0u;
}

void GraphicsBufferDescriptor::Destroy() {}

bool GraphicsBufferDescriptor::IsDirty() const
{
    return IGraphicsResourceDescriptor::IsDirty();
}

void GraphicsBufferDescriptor::SetNativeBuffer(uint32_t buffer)
{
    m_bufferID = buffer;
}

uint32_t GraphicsBufferDescriptor::GetNativeBuffer() const
{
    return m_bufferID;
}

bool GraphicsBufferDescriptor::UpdateData(const void* data, size_t size)
{
    if (IsBuild()) {
        SetSize(size);
        return GetGraphicsAPI()->UpdateGraphicsBufferData(this, data);
    }
    return false;
}

bool GraphicsBufferDescriptor::build()
{
    return GetGraphicsAPI()->BuildGraphicsBuffer(this);
}

} // namespace CS
