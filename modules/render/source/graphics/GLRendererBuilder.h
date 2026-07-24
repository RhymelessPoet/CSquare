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

    // Configure the engine to create its own independent HGLRC on the supplied
    // native window (HWND). If sharedHGLRC is non-null, the new context shares
    // texture/buffer/shader objects with it via wglShareLists.
    GLRendererBuilder& SetNativeWindow(void* hwnd, void* sharedHGLRC);

    std::unique_ptr<Renderer> Build(std::shared_ptr<GraphicsResourceCache> resourceCache) const;

private:
    std::shared_ptr<OpenGLContext> m_sharedContext;
    void* m_hwnd{nullptr};
    void* m_sharedHGLRC{nullptr};
    bool m_useNativeWindow{false};
};

} // namespace CS
