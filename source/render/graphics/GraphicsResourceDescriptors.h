#pragma once
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

    void SetSize(const Size2u& size);
    Size2u GetSize() const { return m_size; }

    const char* GetData() const { return m_data; }
    void SetData(const char* data) { m_data = data; }

    bool IsExternal() const { return m_isExternal; }

protected:
    virtual bool build() override;

private:
    uint32_t m_textureID{0u};
    Size2u m_size;
    const char* m_data{nullptr};
    bool m_isExternal{false};
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
    TextureDescriptor* GetDepthAttachment() const { return m_depthAttachment; }

    void SetDepthAttachment(size_t textureResourceID);

    void SetSize(const Size2u& size);
    const Size2u& GetSize() const { return m_size; }

protected:
    virtual bool build() override;

private:
    Size2u m_size;
    TextureDescriptor* m_colorAttachment{nullptr};
    TextureDescriptor* m_depthAttachment{nullptr};
    std::optional<uint32_t> m_FBO;
};

} // namespace CS
