#pragma once
#include "GraphicsResourceDescriptors.h"
#include "OpenGLContext.h"

namespace CS
{
class GraphicsResourceCache;
class GraphicsGLImpl
{
public:
    static void GlobalInit();
    GraphicsGLImpl(std::unique_ptr<OpenGLContext> context, std::shared_ptr<GraphicsResourceCache> resourceCache);
    ~GraphicsGLImpl();

    GraphicsGLImpl(const GraphicsGLImpl&) = delete;

    bool BuildRenderTarget(RenderTargetDescriptor* descriptor);
    bool BuildTexture(TextureDescriptor* descriptor);
    bool UpdateTextureData(TextureDescriptor* descriptor);
    bool DestroyTexture(TextureDescriptor* descriptor);

    std::shared_ptr<GraphicsResourceCache> GetResourceCache();

private:
    std::unique_ptr<OpenGLContext> m_glContext;
    std::weak_ptr<GraphicsResourceCache> m_resouceCache;
};

} // namespace CS
