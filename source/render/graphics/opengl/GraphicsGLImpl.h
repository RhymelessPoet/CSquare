#pragma once
#include "GraphicsResourceDescriptors.h"
#include "OpenGLContext.h"
#include "base/Color.h"

namespace CS
{
class GraphicsResourceCache;
class IGraphicsResourceDescriptor;
class GraphicsGLImpl
{
public:
    GraphicsGLImpl(std::unique_ptr<OpenGLContext> context, std::shared_ptr<GraphicsResourceCache> resourceCache);
    ~GraphicsGLImpl();

    GraphicsGLImpl(const GraphicsGLImpl&) = delete;

    bool Initialize();

    bool BindRenderTarget(RenderTargetDescriptor* descriptor);
    bool BuildRenderTarget(RenderTargetDescriptor* descriptor);
    bool BuildTexture(TextureDescriptor* descriptor);
    bool UpdateTextureData(TextureDescriptor* descriptor);
    bool DestroyTexture(TextureDescriptor* descriptor);
    bool Clear(std::optional<Color> color, std::optional<float> depth = std::nullopt);

    std::shared_ptr<GraphicsResourceCache> GetResourceCache();

    template <typename DescriptorType>
    DescriptorType* GetResourceDescriptor(size_t id)
    {
        return dynamic_cast<DescriptorType*>(getIResourceDescriptor(id));
    }

private:
    IGraphicsResourceDescriptor* getIResourceDescriptor(size_t id);

private:
    std::unique_ptr<OpenGLContext> m_glContext;
    std::weak_ptr<GraphicsResourceCache> m_resouceCache;
};

} // namespace CS
