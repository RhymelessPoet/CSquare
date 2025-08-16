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

    OpenGLContext& GLViewport(GLint x, GLint y, GLsizei width, GLsizei height);
    OpenGLContext& GLDisable(GLenum cap);
    OpenGLContext& GLEnable(GLenum cap);

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

    OpenGLContext& GLGenBuffers(GLsizei n, GLuint* buffers);
    OpenGLContext& GLBindBuffer(GLenum target, GLuint buffer);
    OpenGLContext& GLBufferData(GLenum target, GLsizeiptr size, const void* data, GLenum usage);
    OpenGLContext& GLDeleteBuffers(GLsizei n, const GLuint* buffers);

    OpenGLContext& GLBindVertexArray(GLuint array);
    OpenGLContext& GLEnableVertexAttribArray(GLuint index);
    OpenGLContext& GLVertexAttribPointer(GLuint index,
                                         GLint size,
                                         GLenum type,
                                         GLboolean normalized,
                                         GLsizei stride,
                                         const void* pointer);
    OpenGLContext& GLGenVertexArrays(GLsizei n, GLuint* arrays);
    OpenGLContext& GLDeleteVertexArrays(GLsizei n, const GLuint* arrays);

    OpenGLContext& GLShaderSource(GLuint shader, GLsizei count, const GLchar* const* str, const GLint* length);
    OpenGLContext& GLCompileShader(GLuint shader);
    OpenGLContext& GLGetShaderiv(GLuint shader, GLenum pname, GLint* params);
    OpenGLContext& GLGetShaderInfoLog(GLuint shader, GLsizei bufSize, GLsizei* length, GLchar* infoLog);
    OpenGLContext& GLCreateShader(GLenum type, GLuint* shader);
    OpenGLContext& GLDeleteShader(GLuint shader);
    OpenGLContext& GLCreateProgram(GLuint* program);
    OpenGLContext& GLAttachShader(GLuint program, GLuint shader);
    OpenGLContext& GLLinkProgram(GLuint program);
    OpenGLContext& GLGetProgramiv(GLuint program, GLenum pname, GLint* params);
    OpenGLContext& GLGetProgramInfoLog(GLuint program, GLsizei bufSize, GLsizei* length, GLchar* infoLog);
    OpenGLContext& GLUseProgram(GLuint program);
    OpenGLContext& GLActiveTexture(GLenum texture);
    OpenGLContext& GLUniform1i(GLint location, GLint v0);
    OpenGLContext& GLUniform1f(GLint location, GLfloat v0);
    OpenGLContext& GLUniform2f(GLint location, GLfloat v0, GLfloat v1);
    OpenGLContext& GLUniform3f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
    OpenGLContext& GLUniform4f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
    OpenGLContext& GLUniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);

    OpenGLContext& GLClearColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
    OpenGLContext& GLClearDepth(GLfloat depth);
    OpenGLContext& GLClear(GLbitfield mask);

    OpenGLContext& GLDrawArrays(GLenum mode, GLint first, GLsizei count);
    OpenGLContext& GLDrawElements(GLenum mode, GLsizei count, GLenum type, const void* indices);

private:
    std::unique_ptr<INativeContext> m_nativeContext;
};

} // namespace CS
