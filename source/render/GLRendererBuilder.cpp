#include "GLRendererBuilder.h"
#include "Renderer.h"
#include "graphics/GraphicsAPI.h"
#include "graphics/opengl/GraphicsGLImpl.h"
#include "graphics/opengl/NativeContext.h"
#include "graphics/opengl/OpenGLContext.h"


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

std::unique_ptr<Renderer> GLRendererBuilder::Build(std::shared_ptr<GraphicsResourceCache> resourceCache) const
{
    auto context = std::make_unique<OpenGLContext>(m_sharedContext);
    auto graphicImpl = std::make_unique<GraphicsGLImpl>(std::move(context), resourceCache);
    auto graphicAPI = GraphicsAPI::Create(std::move(graphicImpl), resourceCache);

    return std::make_unique<Renderer>(graphicAPI);
}

} // namespace CS
