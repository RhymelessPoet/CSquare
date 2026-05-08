#pragma once
#include "base/Size.h"
#include "glad/glad.h"
#include <memory>
#include <string>

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

    // Activate / deactivate the underlying native GL context on the current
    // thread. Must be wrapped around any sequence of GL calls when coexisting
    // with another GL context (e.g. Qt's RHI HGLRC).
    bool MakeCurrent();
    void DoneCurrent();
    void SwapBuffers();

    std::string_view GetVersion();

    OpenGLContext& GLViewport(GLint x, GLint y, GLsizei width, GLsizei height);
    OpenGLContext& GLDisable(GLenum cap);
    OpenGLContext& GLEnable(GLenum cap);
    OpenGLContext& GLDepthFunc(GLenum func);
    OpenGLContext& GLDepthMask(GLboolean flag);

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
    OpenGLContext& GLTexSubImage2D(GLenum target,
                                   GLint level,
                                   GLint xoffset,
                                   GLint yoffset,
                                   GLsizei width,
                                   GLsizei height,
                                   GLenum format,
                                   GLenum type,
                                   const void* pixels);
    OpenGLContext& GLDeleteTextures(GLsizei n, const GLuint* textures);
    OpenGLContext& GLPixelStorei(GLenum pname, GLint param);
    OpenGLContext& GLGenerateMipmap(GLenum target);

    OpenGLContext& GLGenSamplers(GLsizei n, GLuint* samplers);
    OpenGLContext& GLSamplerParameteri(GLuint sampler, GLenum pname, GLint param);
    OpenGLContext& GLDeleteSamplers(GLsizei n, const GLuint* samplers);
    OpenGLContext& GLBindSampler(GLuint unit, GLuint sampler);
    OpenGLContext& GLBindSamplers(GLuint first, GLsizei count, const GLuint* samplers);

    OpenGLContext& GLGenBuffers(GLsizei n, GLuint* buffers);
    OpenGLContext& GLBindBuffer(GLenum target, GLuint buffer);
    OpenGLContext& GLBindBufferBase(GLenum target, GLuint index, GLuint buffer);
    OpenGLContext& GLBindBufferRange(GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size);
    OpenGLContext& GLBufferData(GLenum target, GLsizeiptr size, const void* data, GLenum usage);
    OpenGLContext& GLBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, const void* data);
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
    GLint GLGetUniformLocation(GLuint program, const GLchar* name);

    OpenGLContext& GLClearColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
    OpenGLContext& GLClearDepth(GLfloat depth);
    OpenGLContext& GLClear(GLbitfield mask);

    OpenGLContext& GLDrawArrays(GLenum mode, GLint first, GLsizei count);
    OpenGLContext& GLDrawElements(GLenum mode, GLsizei count, GLenum type, const void* indices);

    OpenGLContext& GLCheck();

private:
    void initVersion();

private:
    std::unique_ptr<INativeContext> m_nativeContext;
    std::string m_version;
};

} // namespace CS
