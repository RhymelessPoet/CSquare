#include "GraphicsInputAssemblyDescriptor.h"
#include "GraphicsBufferDescriptor.h"
#include "graphics/opengl/GraphicsGLImpl.h"

namespace CS
{
GraphicsInputAssemblyDescriptor::GraphicsInputAssemblyDescriptor(size_t id, std::shared_ptr<GraphicsGLImpl> graphicsAPI)
    : IGraphicsResourceDescriptor(id, std::move(graphicsAPI))
{}

bool GraphicsInputAssemblyDescriptor::IsBuild() const
{
    if (m_assemblyData.has_value()) {
        return GetGraphicsAPI()->IsBuild(this);
    }
    return false;
}

void GraphicsInputAssemblyDescriptor::Destroy()
{
    GetGraphicsAPI()->DestroyGraphicsInputAssembly(this);
}

bool GraphicsInputAssemblyDescriptor::IsDirty() const
{
    return IGraphicsResourceDescriptor::IsDirty();
}

void GraphicsInputAssemblyDescriptor::SetVertexInputLayout(std::unique_ptr<VertexInputLayout> vertexInputLayout)
{
    m_vertexInputLayout = std::move(vertexInputLayout);
    setDirty();
}

void GraphicsInputAssemblyDescriptor::SetVertexInput(uint32_t binding, uint32_t bufferResourceID, uint32_t offset)
{
    if (binding >= VertexInputLayout::MaxBindings) {
        return;
    }

    auto& [vertexBuffer, _offset] = m_vertexInputs[binding];
    if (vertexBuffer != nullptr) {
        vertexBuffer->Release();
    }

    auto vertexBufferDesc = GetGraphicsAPI()->GetResourceDescriptor<GraphicsBufferDescriptor>(bufferResourceID);
    vertexBuffer = vertexBufferDesc;

    if (vertexBufferDesc != nullptr) {
        vertexBufferDesc->AddReference();
    }

    _offset = offset;
    setDirty();
}

const GraphicsInputAssemblyDescriptor::VertexInput&
GraphicsInputAssemblyDescriptor::GetVertexInput(uint32_t binding) const
{
    auto index = std::min(binding, VertexInputLayout::MaxBindings);
    return m_vertexInputs[index];
}

void GraphicsInputAssemblyDescriptor::SetIndexBuffer(uint32_t bufferResourceID, IndexFormat format)
{
    if (m_indexBuffer != nullptr) {
        m_indexBuffer->Release();
    }

    m_format = format;
    m_indexBuffer = GetGraphicsAPI()->GetResourceDescriptor<GraphicsBufferDescriptor>(bufferResourceID);

    if (m_indexBuffer != nullptr) {
        m_indexBuffer->AddReference();
    }

    setDirty();
}

bool GraphicsInputAssemblyDescriptor::build()
{
    return GetGraphicsAPI()->BuildGraphicsInputAssembly(this);
}

} // namespace CS
