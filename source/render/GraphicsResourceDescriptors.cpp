#include "GraphicsResourceDescriptors.h"
#include "graphics/opengl/GraphicsGLImpl.h"

namespace CS
{
TextureDescriptor::TextureDescriptor(size_t id, std::shared_ptr<GraphicsGLImpl> graphicsAPI)
    : IGraphicsResourceDescriptor(id, std::move(graphicsAPI))
{}

bool TextureDescriptor::IsBuild() const
{
    return m_textureID != 0u;
}

void TextureDescriptor::Destroy() {}

bool TextureDescriptor::build()
{
    return m_graphicsAPI.lock()->BuildTexture(this);
}

uint32_t TextureDescriptor::GetNativeTexture() const
{
    return m_textureID;
}

void TextureDescriptor::SetNativeTexture(uint32_t texture)
{
    m_textureID = texture;
}

RenderTargetDescriptor::RenderTargetDescriptor(size_t id, std::shared_ptr<GraphicsGLImpl> graphicsAPI)
    : IGraphicsResourceDescriptor(id, std::move(graphicsAPI))
{}

bool RenderTargetDescriptor::IsBuild() const
{
    return m_FBO.has_value();
}

void RenderTargetDescriptor::Destroy() {}

bool RenderTargetDescriptor::build()
{
    return m_graphicsAPI.lock()->BuildRenderTarget(this);
}

void RenderTargetDescriptor::SetNativeFBO(uint32_t fbo)
{
    m_FBO = fbo;
}

std::optional<uint32_t> RenderTargetDescriptor::GetNativeFBO() const
{
    return m_FBO;
}

void RenderTargetDescriptor::SetColorAttachment(TextureDescriptor* descriptor)
{
    if (m_colorAttachment != nullptr) {
        m_colorAttachment->Release();
    }

    m_colorAttachment = descriptor;
    m_colorAttachment->AddReference();
    setDirty();
}

void RenderTargetDescriptor::SetDepthAttachment(TextureDescriptor* descriptor)
{
    if (m_depthAttachment != nullptr) {
        m_depthAttachment->Release();
    }
    m_depthAttachment = descriptor;
    m_depthAttachment->AddReference();
    setDirty();
}

void RenderTargetDescriptor::SetSize(const Size2U& size)
{
    m_size = size;
    setDirty();
}

} // namespace CS
