#include "GLRendererBuilder.h"
#include "graphics/GraphicsAPI.h"
#include "graphics/GraphicsGLImpl.h"
#include "graphics/opengl/NativeContext.h"
#include "graphics/opengl/OpenGLContext.h"
#include "renderer/Renderer.h"

namespace CS
{
GLRendererBuilder& GLRendererBuilder::SetSharedContext(std::shared_ptr<OpenGLContext> context)
{
    m_sharedContext = std::move(context);
    return *this;
}

GLRendererBuilder& GLRendererBuilder::SetSharedContext(void* context)
{
    m_sharedContext =
        std::make_shared<CS::OpenGLContext>(std::make_unique<CS::WGLContext>(reinterpret_cast<HGLRC>(context)));
    return *this;
}

GLRendererBuilder& GLRendererBuilder::SetNativeWindow(void* hwnd, void* sharedHGLRC)
{
    m_hwnd = hwnd;
    m_sharedHGLRC = sharedHGLRC;
    m_useNativeWindow = true;
    return *this;
}

std::unique_ptr<Renderer> GLRendererBuilder::Build(std::shared_ptr<GraphicsResourceCache> resourceCache) const
{
    std::unique_ptr<OpenGLContext> context;
    if (m_useNativeWindow) {
        auto native =
            std::make_unique<WGLWindowContext>(reinterpret_cast<HWND>(m_hwnd), reinterpret_cast<HGLRC>(m_sharedHGLRC));
        native->MakeCurrent();
        context = std::make_unique<OpenGLContext>(std::move(native));
    } else {
        context = std::make_unique<OpenGLContext>(m_sharedContext);
    }
    auto graphicImpl = std::make_unique<GraphicsGLImpl>(std::move(context), resourceCache);
    auto graphicAPI = GraphicsAPI::Create(std::move(graphicImpl), resourceCache);

    return std::make_unique<Renderer>(graphicAPI);
}

} // namespace CS
