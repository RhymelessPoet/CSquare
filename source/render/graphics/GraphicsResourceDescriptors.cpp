#include "GraphicsResourceDescriptors.h"
#include "graphics/GraphicsGLImpl.h"

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
    } else if (IsBuild()) {
        GetGraphicsAPI()->DestroyTexture(this);
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
    return GetGraphicsAPI()->BuildTexture(this);
}

uint32_t TextureDescriptor::GetNativeTexture() const
{
    return m_textureID;
}

void TextureDescriptor::SetNativeTexture(uint32_t texture)
{
    m_textureID = texture;
}

void TextureDescriptor::SetExternalTexture(uint32_t textureID)
{
    m_textureID = textureID;
    m_isExternal = true;
}

void TextureDescriptor::SetSize(const Size2u& size, bool toSetDirty)
{
    m_size = size;
    if (toSetDirty) {
        setDirty();
    }
}

void TextureDescriptor::SetFormat(TextureFormat format)
{
    m_format = format;
    setDirty();
}

void TextureDescriptor::SetMipmap(bool mipmap)
{
    m_isMipmap = mipmap;
    setDirty();
}

void TextureDescriptor::UpdateData(const void* data)
{
    auto graphicsAPI = GetGraphicsAPI();
    if (graphicsAPI != nullptr && !m_isExternal) {
        graphicsAPI->UpdateTextureData(this, data);
    }
}

RenderTargetDescriptor::RenderTargetDescriptor(size_t id, std::shared_ptr<GraphicsGLImpl> graphicsAPI)
    : IGraphicsResourceDescriptor(id, std::move(graphicsAPI))
{}

bool RenderTargetDescriptor::IsBuild() const
{
    return m_FBO.has_value();
}

void RenderTargetDescriptor::Destroy()
{
    GetGraphicsAPI()->DestroyRenderTarget(this);
}

bool RenderTargetDescriptor::IsDirty() const
{
    bool colorAttachmentDirty = m_colorAttachment != nullptr && m_colorAttachment->IsDirty();
    bool depthAttachmentDirty = m_depthStencilAttachment != nullptr && m_depthStencilAttachment->IsDirty();

    return IGraphicsResourceDescriptor::IsDirty() || colorAttachmentDirty || depthAttachmentDirty;
}

bool RenderTargetDescriptor::build()
{
    return GetGraphicsAPI()->BuildRenderTarget(this);
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
    auto graphicsAPI = GetGraphicsAPI();
    if (m_colorAttachment != nullptr) {
        m_colorAttachment->Release();
    }

    m_colorAttachment = graphicsAPI->GetResourceDescriptor<TextureDescriptor>(textureResourceID);
    m_colorAttachment->AddReference();
    setDirty();
}

void RenderTargetDescriptor::SetDepthStencilAttachment(size_t textureResourceID)
{
    auto graphicsAPI = GetGraphicsAPI();
    if (m_depthStencilAttachment != nullptr) {
        m_depthStencilAttachment->Release();
    }

    m_depthStencilAttachment = graphicsAPI->GetResourceDescriptor<TextureDescriptor>(textureResourceID);
    m_depthStencilAttachment->AddReference();
    setDirty();
}

void RenderTargetDescriptor::SetSize(const Size2u& size)
{
    m_size = size;
    if (m_colorAttachment != nullptr) {
        m_colorAttachment->SetSize(size);
    }
    if (m_depthStencilAttachment != nullptr) {
        m_depthStencilAttachment->SetSize(size);
    }
    setDirty();
}

SamplerDescriptor::SamplerDescriptor(size_t id, std::shared_ptr<GraphicsGLImpl> graphicsAPI)
    : IGraphicsResourceDescriptor(id, std::move(graphicsAPI))
{}

bool SamplerDescriptor::IsBuild() const
{
    return m_samplerID != 0u;
}

void SamplerDescriptor::Destroy() {}

bool SamplerDescriptor::IsDirty() const
{
    return IGraphicsResourceDescriptor::IsDirty();
}

uint32_t SamplerDescriptor::GetNativeSampler() const
{
    return m_samplerID;
}

void SamplerDescriptor::SetNativeSampler(uint32_t sampler)
{
    m_samplerID = sampler;
}

void SamplerDescriptor::SetAddressModeUV(AddressMode u, AddressMode v)
{
    m_u = u;
    m_v = v;
    setDirty();
}

void SamplerDescriptor::SetAddressModeW(AddressMode w)
{
    m_w = w;
    setDirty();
}

void SamplerDescriptor::SetFilter(FilterMode min, FilterMode mag)
{
    m_minFilter = min;
    m_magFilter = mag;
    setDirty();
}

void SamplerDescriptor::SetMipmapFilter(MipmapFilterMode mode)
{
    m_mipmapFilter = mode;
    setDirty();
}

bool SamplerDescriptor::build()
{
    auto graphicsAPI = GetGraphicsAPI();
    return graphicsAPI->BuildSampler(this);
}

} // namespace CS
