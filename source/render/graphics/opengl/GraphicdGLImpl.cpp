#include "GraphicsGLImpl.h"
#include "graphics/GraphicsResourceCache.h"
#include "graphics/opengl/glad/include/glad/glad.h"

namespace CS
{

GraphicsGLImpl::GraphicsGLImpl(std::unique_ptr<OpenGLContext> context,
                               std::shared_ptr<GraphicsResourceCache> resourceCache)
    : m_glContext(std::move(context)), m_resouceCache(resourceCache)
{}

GraphicsGLImpl::~GraphicsGLImpl() {}

bool GraphicsGLImpl::Initialize()
{
    return gladLoadGL() == 1;
}

bool GraphicsGLImpl::BindRenderTarget(RenderTargetDescriptor* descriptor)
{
    if (descriptor->m_FBO.has_value()) {
        m_glContext->GLBindFramebuffer(GL_FRAMEBUFFER, descriptor->m_FBO.value());
        return true;
    }
    return false;
}

bool GraphicsGLImpl::BuildRenderTarget(RenderTargetDescriptor* descriptor)
{
    GLuint fbo = 0u;

    bool hasColorAttachment = false;
    bool hasDepthAttachment = false;

    auto colorTexture = descriptor->m_colorAttachment;
    if (colorTexture != nullptr) {
        hasColorAttachment = colorTexture->Build();
    }
    auto depthTexture = descriptor->m_depthAttachment;
    if (depthTexture != nullptr) {
        hasDepthAttachment = depthTexture->Build();
    }

    m_glContext->GLGenFramebuffers(1, &fbo).GLBindFramebuffer(GL_FRAMEBUFFER, fbo);

    if (hasColorAttachment) {
        m_glContext->GLFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                                            colorTexture->GetNativeTexture(), 0);
    }
    if (hasDepthAttachment) {
        m_glContext->GLFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
                                            depthTexture->GetNativeTexture(), 0);
    }
    bool condition = m_glContext->GLCheckFramebufferStatus(GL_FRAMEBUFFER);

    if (condition) {
        descriptor->SetNativeFBO(fbo);
    } else {
        m_glContext->GLDeleteFramebuffers(1, &fbo);
    }
    m_glContext->GLBindFramebuffer(GL_FRAMEBUFFER, 0);

    return condition;
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

IGraphicsResourceDescriptor* GraphicsGLImpl::getIResourceDescriptor(size_t id)
{
    return GetResourceCache()->GetIDescriptor(id);
}

} // namespace CS
