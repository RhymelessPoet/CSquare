#pragma once
#include "GraphicsResourceParameters.h"
#include "IGraphicsResourceDescriptor.h"
#include "base/Size.h"
#include <cstdint>
#include <optional>

namespace CS
{

class TextureDescriptor final : public IGraphicsResourceDescriptor
{
public:
    TextureDescriptor(size_t id, std::shared_ptr<GraphicsGLImpl> graphicsAPI);

    virtual bool IsBuild() const override;
    virtual void Destroy() override;
    virtual bool IsDirty() const override;

    uint32_t GetNativeTexture() const;

    void SetNativeTexture(uint32_t texture);
    void SetExternalTexture(uint32_t textureID);

    void SetSize(const Size2u& size, bool toSetDirty = true);
    Size2u GetSize() const { return m_size; }

    void SetFormat(TextureFormat format);
    TextureFormat GetFormat() const { return m_format; }

    void SetMipmap(bool mipmap);
    bool IsMipmap() const { return m_isMipmap; }

    void UpdateData(const void* data);

    bool IsExternal() const { return m_isExternal; }

protected:
    virtual bool build() override;

private:
    Size2u m_size;
    uint32_t m_textureID{0u};
    TextureFormat m_format{TextureFormat::RGBA8Unorm};
    bool m_isMipmap{false};
    bool m_isExternal{false};
};

class SamplerDescriptor final : public IGraphicsResourceDescriptor
{
public:
    SamplerDescriptor(size_t id, std::shared_ptr<GraphicsGLImpl> graphicsAPI);

    virtual bool IsBuild() const override;
    virtual void Destroy() override;
    virtual bool IsDirty() const override;

    uint32_t GetNativeSampler() const;
    void SetNativeSampler(uint32_t sampler);

    void SetAddressModeUV(AddressMode u, AddressMode v);
    void SetAddressModeW(AddressMode w);
    std::pair<AddressMode, AddressMode> GetAddressModeUV() const { return {m_u, m_v}; }
    AddressMode GetAddressModeW() const { return m_w; }

    void SetFilter(FilterMode min, FilterMode mag);
    std::pair<FilterMode, FilterMode> GetFilterMode() const { return {m_minFilter, m_magFilter}; }

    void SetMipmapFilter(MipmapFilterMode mode);
    MipmapFilterMode GetMipmapFilter() const { return m_mipmapFilter; }

protected:
    virtual bool build() override;

private:
    uint32_t m_samplerID{0u};
    AddressMode m_u{AddressMode::Repeat};
    AddressMode m_v{AddressMode::Repeat};
    AddressMode m_w{AddressMode::Repeat};
    FilterMode m_minFilter{FilterMode::Linear};
    FilterMode m_magFilter{FilterMode::Linear};
    MipmapFilterMode m_mipmapFilter{MipmapFilterMode::Max};
};

class RenderTargetDescriptor final : public IGraphicsResourceDescriptor
{
public:
    RenderTargetDescriptor(size_t id, std::shared_ptr<GraphicsGLImpl> graphicsAPI);

    virtual bool IsBuild() const override;
    virtual void Destroy() override;

    virtual bool IsDirty() const override;

    void SetNativeFBO(uint32_t fbo);
    std::optional<uint32_t> GetNativeFBO() const;

    void SetColorAttachment(size_t textureResourceID);
    TextureDescriptor* GetColorAttachment() const { return m_colorAttachment; }

    void SetDepthStencilAttachment(size_t textureResourceID);
    TextureDescriptor* GetDepthStencilAttachment() const { return m_depthStencilAttachment; }

    void SetDepthAttachment(size_t textureResourceID);
    TextureDescriptor* GetDepthAttachment() const { return m_depthAttachment; }

    void SetSize(const Size2u& size);
    const Size2u& GetSize() const { return m_size; }

protected:
    virtual bool build() override;

private:
    Size2u m_size;
    TextureDescriptor* m_colorAttachment{nullptr};
    TextureDescriptor* m_depthAttachment{nullptr};
    TextureDescriptor* m_depthStencilAttachment{nullptr};
    std::optional<uint32_t> m_FBO;
};

} // namespace CS
