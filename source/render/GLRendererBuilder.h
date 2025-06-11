#pragma once
#include <memory>

namespace CS
{
class OpenGLContext;
class Renderer;
class GraphicsResourceCache;
class GLRendererBuilder
{
public:
    GLRendererBuilder() = default;
    ~GLRendererBuilder() = default;

    GLRendererBuilder& SetSharedContext(std::shared_ptr<OpenGLContext> context);
    GLRendererBuilder& SetSharedContext(void* context);

    std::unique_ptr<Renderer> Build(std::shared_ptr<GraphicsResourceCache> resourceCache) const;

private:
    std::shared_ptr<OpenGLContext> m_sharedContext;
};

} // namespace CS
