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

    // template <typename NativeContextType>
    // bool Is() const
    // {
    //     return m_nativeContext.type() == typeid(NativeContextType);
    // }

    // template <typename NativeContextHandleType>
    // NativeContextHandleType GetNativeContext() const
    // {
    //     return std::any_cast<NativeContextType>(m_nativeContext);
    // }

    bool IsShared() const;

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
    OpenGLContext& GLClear(GLbitfield mask);

private:
    std::unique_ptr<INativeContext> m_nativeContext;
};

} // namespace CS
