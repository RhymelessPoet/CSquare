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
    friend class GraphicsGLImpl;

    TextureDescriptor(size_t id, std::shared_ptr<GraphicsGLImpl> graphicsAPI);

    virtual bool IsBuild() const override;
    virtual void Destroy() override;

    uint32_t GetNativeTexture() const;

    void SetNativeTexture(uint32_t texture);

protected:
    virtual bool build() override;

private:
    uint32_t m_textureID{0u};
    Size2U m_size;
    const char* m_data{nullptr};
};

class RenderTargetDescriptor final : public IGraphicsResourceDescriptor
{
public:
    friend class GraphicsGLImpl;

    RenderTargetDescriptor(size_t id, std::shared_ptr<GraphicsGLImpl> graphicsAPI);

    virtual bool IsBuild() const override;
    virtual void Destroy() override;

    virtual bool IsDirty() const override;

    void SetNativeFBO(uint32_t fbo);
    std::optional<uint32_t> GetNativeFBO() const;

    void SetColorAttachment(size_t textureResourceID);

    void SetDepthAttachment(size_t textureResourceID);

    void SetSize(const Size2U& size);

protected:
    virtual bool build() override;

private:
    Size2U m_size;
    TextureDescriptor* m_colorAttachment{nullptr};
    TextureDescriptor* m_depthAttachment{nullptr};
    std::optional<uint32_t> m_FBO;
};

} // namespace CS
