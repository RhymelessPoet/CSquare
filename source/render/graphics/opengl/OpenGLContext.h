#pragma once
#include "base/Size.h"
#include "glad/glad.h"
#include <memory>

namespace CS
{
class INativeContext;
class OpenGLContext
{
public:
    OpenGLContext();
    OpenGLContext(std::shared_ptr<OpenGLContext> sharedContext);
    OpenGLContext(std::unique_ptr<INativeContext> nativeContext);
    ~OpenGLContext();

    bool IsShared() const;

    OpenGLContext& GLBindFramebuffer(GLenum target, GLuint framebuffer);
    OpenGLContext& GLGenFramebuffers(GLsizei n, GLuint* ids);
    OpenGLContext& GLDeleteFramebuffers(GLsizei n, const GLuint* framebuffers);
    OpenGLContext&
    GLFramebufferTexture2D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
    bool GLCheckFramebufferStatus(GLenum target);

    OpenGLContext& GLGenTextures(GLsizei n, GLuint* textures);
    OpenGLContext& GLBindTexture(GLenum target, GLuint texture);
    OpenGLContext& GLTexParameteri(GLenum target, GLenum pname, GLint param);
    OpenGLContext& GLTexImage2D(GLenum target,
                                GLint level,
                                GLint internalformat,
                                GLsizei width,
                                GLsizei height,
                                GLint border,
                                GLenum format,
                                GLenum type,
                                const void* pixels);
    OpenGLContext& GLDeleteTextures(GLsizei n, const GLuint* textures);

    OpenGLContext& GLClearColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
    OpenGLContext& GLClearDepth(GLfloat depth);
    OpenGLContext& GLClear(GLbitfield mask);

private:
    std::unique_ptr<INativeContext> m_nativeContext;
};

} // namespace CS
