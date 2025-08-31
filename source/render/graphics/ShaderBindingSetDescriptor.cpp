#include "ShaderBindingSetDescriptor.h"
#include "GraphicsBufferDescriptor.h"
#include "graphics/GraphicsGLImpl.h"
#include <stdexcept>

namespace CS
{
ShaderBindingSetLayoutDescriptor::ShaderBindingSetLayoutDescriptor(size_t id,
                                                                   std::shared_ptr<GraphicsGLImpl> graphicsAPI)
    : IGraphicsResourceDescriptor(id, graphicsAPI)
{}

bool ShaderBindingSetLayoutDescriptor::IsBuild() const
{
    return true;
}

void ShaderBindingSetLayoutDescriptor::Destroy() {}

bool ShaderBindingSetLayoutDescriptor::IsDirty() const
{
    return false;
}

void ShaderBindingSetLayoutDescriptor::AddBinding(const ShaderBinding& binding)
{
    m_bindings.emplace_back(binding);
}

bool ShaderBindingSetLayoutDescriptor::build()
{
    return false;
}

ShaderBindingSetDescriptor::ShaderBindingSetDescriptor(size_t id,
                                                       std::shared_ptr<GraphicsGLImpl> graphicsAPI,
                                                       size_t layoutResourceID)
    : IGraphicsResourceDescriptor(id, graphicsAPI)
{
    setLayout(layoutResourceID);
    m_bindings.resize(m_layout->GetBindings().size());
}

bool ShaderBindingSetDescriptor::IsBuild() const
{
    return true;
}

void ShaderBindingSetDescriptor::Destroy() {}

bool ShaderBindingSetDescriptor::IsDirty() const
{
    return false;
}

bool ShaderBindingSetDescriptor::BindUniformBuffer(size_t binding, size_t bufferID, size_t offset, size_t range)
{
    if (binding >= m_bindings.size()) {
        // log error
        return false;
    }

    UniformBufferBinding ubBinding;
    ubBinding.buffer = GetGraphicsAPI()->GetResourceDescriptor<GraphicsBufferDescriptor>(bufferID);
    ubBinding.offset = offset;
    ubBinding.range = range;

    m_bindings[binding] = ubBinding;
    setDirty();

    return true;
}

void ShaderBindingSetDescriptor::setLayout(size_t layoutResourceID)
{
    auto graphicsAPI = GetGraphicsAPI();

    auto layout = graphicsAPI->GetResourceDescriptor<ShaderBindingSetLayoutDescriptor>(layoutResourceID);

    if (m_layout != nullptr) {
        m_layout->Release();
    }
    m_layout = layout;
    m_layout->AddReference();
}

bool ShaderBindingSetDescriptor::build()
{
    return true;
}

} // namespace CS
