#include "GraphicsResourceDescriptors.h"
#include "graphics/opengl/GraphicsGLImpl.h"

namespace CS
{
TextureDescriptor::TextureDescriptor(size_t id, std::shared_ptr<GraphicsGLImpl> graphicsAPI)
    : IGraphicsResourceDescriptor(id, std::move(graphicsAPI))
{}

bool TextureDescriptor::IsBuild() const
{
    return m_isExternal || m_textureID != 0u;
}

void TextureDescriptor::Destroy()
{
    if (m_isExternal) {
        m_textureID = 0u;
        m_isExternal = false;
    } else {
        m_graphicsAPI.lock()->DestroyTexture(this);
    }
}

bool TextureDescriptor::IsDirty() const
{
    return IGraphicsResourceDescriptor::IsDirty() && !m_isExternal;
}

bool TextureDescriptor::build()
{
    if (m_isExternal) {
        return true; // If it's an external texture, we assume it's already built.
    }
    return m_graphicsAPI.lock()->BuildTexture(this);
}

uint32_t TextureDescriptor::GetNativeTexture() const
{
    return m_textureID;
}

void TextureDescriptor::SetNativeTexture(uint32_t texture)
{
    m_textureID = texture;
    m_isExternal = true;
}

void TextureDescriptor::SetSize(const Size2U& size)
{
    m_size = size;
    setDirty();
}

RenderTargetDescriptor::RenderTargetDescriptor(size_t id, std::shared_ptr<GraphicsGLImpl> graphicsAPI)
    : IGraphicsResourceDescriptor(id, std::move(graphicsAPI))
{}

bool RenderTargetDescriptor::IsBuild() const
{
    return m_FBO.has_value();
}

void RenderTargetDescriptor::Destroy() {}

bool RenderTargetDescriptor::IsDirty() const
{
    bool colorAttachmentDirty = m_colorAttachment != nullptr && m_colorAttachment->IsDirty();
    bool depthAttachmentDirty = m_depthAttachment != nullptr && m_depthAttachment->IsDirty();

    return IGraphicsResourceDescriptor::IsDirty() || colorAttachmentDirty || depthAttachmentDirty;
}

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

void RenderTargetDescriptor::SetColorAttachment(size_t textureResourceID)
{
    auto graphicsAPI = m_graphicsAPI.lock();
    if (m_colorAttachment != nullptr) {
        m_colorAttachment->Release();
    }

    m_colorAttachment = graphicsAPI->GetResourceDescriptor<TextureDescriptor>(textureResourceID);
    m_colorAttachment->AddReference();
    setDirty();
}

void RenderTargetDescriptor::SetDepthAttachment(size_t textureResourceID)
{
    auto graphicsAPI = m_graphicsAPI.lock();
    if (m_depthAttachment != nullptr) {
        m_depthAttachment->Release();
    }

    m_depthAttachment = graphicsAPI->GetResourceDescriptor<TextureDescriptor>(textureResourceID);
    m_depthAttachment->AddReference();
    setDirty();
}

void RenderTargetDescriptor::SetSize(const Size2U& size)
{
    m_size = size;
    if (m_colorAttachment != nullptr) {
        m_colorAttachment->SetSize(size);
    }
    if (m_depthAttachment != nullptr) {
        m_depthAttachment->SetSize(size);
    }
    setDirty();
}

} // namespace CS
