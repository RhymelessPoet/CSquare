#include "OpenGLContext.h"
#include "NativeContext.h"

namespace CS
{

OpenGLContext::OpenGLContext()
{
    m_nativeContext = std::make_unique<GLFWContext>();
}

OpenGLContext::OpenGLContext(std::shared_ptr<OpenGLContext> sharedContext) {}

OpenGLContext::OpenGLContext(std::unique_ptr<INativeContext> nativeContext) : m_nativeContext(std::move(nativeContext))
{}

OpenGLContext::~OpenGLContext() {}

bool OpenGLContext::IsShared() const
{
    return false;
}

OpenGLContext& OpenGLContext::GLGenTextures(GLsizei n, GLuint* textures)
{
    glGenTextures(n, textures);
    return *this;
}

OpenGLContext& OpenGLContext::GLBindTexture(GLenum target, GLuint texture)
{
    glBindTexture(target, texture);
    return *this;
}

OpenGLContext& OpenGLContext::GLTexParameteri(GLenum target, GLenum pname, GLint param)
{
    glTexParameteri(target, pname, param);
    return *this;
}

OpenGLContext& OpenGLContext::GLTexImage2D(GLenum target,
                                           GLint level,
                                           GLint internalformat,
                                           GLsizei width,
                                           GLsizei height,
                                           GLint border,
                                           GLenum format,
                                           GLenum type,
                                           const void* pixels)
{
    glTexImage2D(target, level, internalformat, width, height, border, format, type, pixels);
    return *this;
}

OpenGLContext& OpenGLContext::GLDeleteTextures(GLsizei n, const GLuint* textures)
{
    glDeleteTextures(n, textures);
    return *this;
}

OpenGLContext& OpenGLContext::GLClearColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
    glClearColor(red, green, blue, alpha);
    return *this;
}

OpenGLContext& OpenGLContext::GLClear(GLbitfield mask)
{
    glClear(mask);
    return *this;
}

} // namespace CS
