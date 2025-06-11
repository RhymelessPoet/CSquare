#include "GraphicsGLImpl.h"
#include "graphics/opengl/glad/include/glad/glad.h"

namespace CS
{
void GraphicsGLImpl::GlobalInit()
{
    gladLoadGL();
}

GraphicsGLImpl::GraphicsGLImpl(std::unique_ptr<OpenGLContext> context,
                               std::shared_ptr<GraphicsResourceCache> resourceCache)
    : m_glContext(std::move(context)), m_resouceCache(resourceCache)
{}

GraphicsGLImpl::~GraphicsGLImpl() {}

bool GraphicsGLImpl::BuildRenderTarget(RenderTargetDescriptor* descriptor)
{

    return false;
}

bool GraphicsGLImpl::BuildTexture(TextureDescriptor* descriptor)
{
    GLuint textureID = 0u;

    m_glContext->GLGenTextures(1, &textureID)
        .GLBindTexture(GL_TEXTURE_2D, textureID)
        .GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR)
        .GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR)
        .GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT)
        .GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT)
        .GLTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, descriptor->m_size.Width(), descriptor->m_size.Height(), 0, GL_RGBA,
                      GL_UNSIGNED_BYTE, descriptor->m_data)
        .GLBindTexture(GL_TEXTURE_2D, 0);
    descriptor->m_textureID = textureID;
    return true;
}

bool GraphicsGLImpl::UpdateTextureData(TextureDescriptor* descriptor)
{
    m_glContext->GLBindTexture(GL_TEXTURE_2D, descriptor->m_textureID)
        .GLTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, descriptor->m_size.Width(), descriptor->m_size.Height(), 0, GL_RGBA,
                      GL_UNSIGNED_BYTE, descriptor->m_data)
        .GLBindTexture(GL_TEXTURE_2D, 0);

    return true;
}

bool GraphicsGLImpl::DestroyTexture(TextureDescriptor* descriptor)
{
    m_glContext->GLBindTexture(GL_TEXTURE_2D, 0).GLDeleteTextures(1, &descriptor->m_textureID);

    return true;
}

std::shared_ptr<GraphicsResourceCache> GraphicsGLImpl::GetResourceCache()
{
    return m_resouceCache.lock();
}

} // namespace CS
