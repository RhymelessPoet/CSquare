#include "OpenGLContext.h"
#include "NativeContext.h"
#include <iostream>

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

OpenGLContext& OpenGLContext::GLViewport(GLint x, GLint y, GLsizei width, GLsizei height)
{
    glViewport(x, y, width, height);
    return *this;
}

OpenGLContext& OpenGLContext::GLDisable(GLenum cap)
{
    glDisable(cap);
    return *this;
}

OpenGLContext& OpenGLContext::GLEnable(GLenum cap)
{
    glEnable(cap);
    return *this;
}

OpenGLContext& OpenGLContext::GLDepthFunc(GLenum func)
{
    glDepthFunc(func);
    return *this;
}

OpenGLContext& OpenGLContext::GLDepthMask(GLboolean flag)
{
    glDepthMask(flag);
    return *this;
}

OpenGLContext& OpenGLContext::GLBindFramebuffer(GLenum target, GLuint framebuffer)
{
    glBindFramebuffer(target, framebuffer);
    return *this;
}

OpenGLContext& OpenGLContext::GLGenFramebuffers(GLsizei n, GLuint* ids)
{
    glGenFramebuffers(n, ids);
    return *this;
}

OpenGLContext& OpenGLContext::GLDeleteFramebuffers(GLsizei n, const GLuint* framebuffers)
{
    glDeleteFramebuffers(n, framebuffers);
    return *this;
}

OpenGLContext&
OpenGLContext::GLFramebufferTexture2D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)
{
    glFramebufferTexture2D(target, attachment, textarget, texture, level);
    return *this;
}

bool OpenGLContext::GLCheckFramebufferStatus(GLenum target)
{
    return GL_FRAMEBUFFER_COMPLETE == glCheckFramebufferStatus(target);
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

OpenGLContext& OpenGLContext::GLGenSamplers(GLsizei n, GLuint* samplers)
{
    glGenSamplers(n, samplers);
    return *this;
}

OpenGLContext& OpenGLContext::GLDeleteSamplers(GLsizei n, const GLuint* samplers)
{
    glDeleteSamplers(n, samplers);
    return *this;
}

OpenGLContext& OpenGLContext::GLBindSampler(GLuint unit, GLuint sampler)
{
    glBindSampler(unit, sampler);
    return *this;
}

OpenGLContext& OpenGLContext::GLBindSamplers(GLuint first, GLsizei count, const GLuint* samplers)
{
    glBindSamplers(first, count, samplers);
    return *this;
}

OpenGLContext& OpenGLContext::GLSamplerParameteri(GLuint sampler, GLenum pname, GLint param)
{
    glSamplerParameteri(sampler, pname, param);
    return *this;
}

OpenGLContext& OpenGLContext::GLGenBuffers(GLsizei n, GLuint* buffers)
{
    glGenBuffers(n, buffers);
    return *this;
}

OpenGLContext& OpenGLContext::GLBindBuffer(GLenum target, GLuint buffer)
{
    glBindBuffer(target, buffer);
    return *this;
}

OpenGLContext& OpenGLContext::GLBindBufferBase(GLenum target, GLuint index, GLuint buffer)
{
    glBindBufferBase(target, index, buffer);
    return *this;
}

OpenGLContext&
OpenGLContext::GLBindBufferRange(GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size)
{
    GLint alignment;
    glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &alignment);
    if (offset % alignment != 0) {
        std::cerr << "offset not align " << alignment << std::endl;
    }
    glBindBufferRange(target, index, buffer, offset, size);
    return *this;
}

OpenGLContext& OpenGLContext::GLBufferData(GLenum target, GLsizeiptr size, const void* data, GLenum usage)
{
    glBufferData(target, size, data, usage);
    return *this;
}

OpenGLContext& OpenGLContext::GLBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, const void* data)
{
    GLint alignment;
    glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &alignment);
    if (offset % alignment != 0) {
        std::cerr << "offset not align " << alignment << std::endl;
    }
    glBufferSubData(target, offset, size, data);
    return *this;
}

OpenGLContext& OpenGLContext::GLDeleteBuffers(GLsizei n, const GLuint* buffers)
{
    glDeleteBuffers(n, buffers);
    return *this;
}

OpenGLContext& OpenGLContext::GLBindVertexArray(GLuint array)
{
    glBindVertexArray(array);
    return *this;
}

OpenGLContext& OpenGLContext::GLEnableVertexAttribArray(GLuint index)
{
    glEnableVertexAttribArray(index);
    return *this;
}

OpenGLContext& OpenGLContext::GLVertexAttribPointer(GLuint index,
                                                    GLint size,
                                                    GLenum type,
                                                    GLboolean normalized,
                                                    GLsizei stride,
                                                    const void* pointer)
{
    glVertexAttribPointer(index, size, type, normalized, stride, pointer);
    return *this;
}

OpenGLContext& OpenGLContext::GLGenVertexArrays(GLsizei n, GLuint* arrays)
{
    glGenVertexArrays(n, arrays);
    return *this;
}

OpenGLContext& OpenGLContext::GLDeleteVertexArrays(GLsizei n, const GLuint* arrays)
{
    glDeleteVertexArrays(n, arrays);
    return *this;
}

OpenGLContext&
OpenGLContext::GLShaderSource(GLuint shader, GLsizei count, const GLchar* const* str, const GLint* length)
{
    glShaderSource(shader, count, str, length);
    return *this;
}

OpenGLContext& OpenGLContext::GLCompileShader(GLuint shader)
{
    glCompileShader(shader);

    GLint status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE) {
        GLint logLength;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
        GLchar* infoLog = new GLchar[logLength];
        glGetShaderInfoLog(shader, logLength, nullptr, infoLog);
        delete[] infoLog;
    }

    return *this;
}

OpenGLContext& OpenGLContext::GLGetShaderiv(GLuint shader, GLenum pname, GLint* params)
{
    glGetShaderiv(shader, pname, params);
    return *this;
}

OpenGLContext& OpenGLContext::GLGetShaderInfoLog(GLuint shader, GLsizei bufSize, GLsizei* length, GLchar* infoLog)
{
    glGetShaderInfoLog(shader, bufSize, length, infoLog);
    return *this;
}

OpenGLContext& OpenGLContext::GLCreateShader(GLenum type, GLuint* shader)
{
    *shader = glCreateShader(type);
    return *this;
}

OpenGLContext& OpenGLContext::GLDeleteShader(GLuint shader)
{
    glDeleteShader(shader);
    return *this;
}

OpenGLContext& OpenGLContext::GLCreateProgram(GLuint* program)
{
    *program = glCreateProgram();
    return *this;
}

OpenGLContext& OpenGLContext::GLAttachShader(GLuint program, GLuint shader)
{
    glAttachShader(program, shader);
    return *this;
}

OpenGLContext& OpenGLContext::GLLinkProgram(GLuint program)
{
    glLinkProgram(program);
    return *this;
}

OpenGLContext& OpenGLContext::GLGetProgramiv(GLuint program, GLenum pname, GLint* params)
{
    glGetProgramiv(program, pname, params);
    return *this;
}

OpenGLContext& OpenGLContext::GLGetProgramInfoLog(GLuint program, GLsizei bufSize, GLsizei* length, GLchar* infoLog)
{
    glGetProgramInfoLog(program, bufSize, length, infoLog);
    return *this;
}

OpenGLContext& OpenGLContext::GLUseProgram(GLuint program)
{
    glUseProgram(program);
    return *this;
}

OpenGLContext& OpenGLContext::GLActiveTexture(GLenum texture)
{
    glActiveTexture(texture);
    return *this;
}

OpenGLContext& OpenGLContext::GLUniform1i(GLint location, GLint v0)
{
    glUniform1i(location, v0);
    return *this;
}

OpenGLContext& OpenGLContext::GLUniform1f(GLint location, GLfloat v0)
{
    glUniform1f(location, v0);
    return *this;
}

OpenGLContext& OpenGLContext::GLUniform2f(GLint location, GLfloat v0, GLfloat v1)
{
    glUniform2f(location, v0, v1);
    return *this;
}

OpenGLContext& OpenGLContext::GLUniform3f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2)
{
    glUniform3f(location, v0, v1, v2);
    return *this;
}

OpenGLContext& OpenGLContext::GLUniform4f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3)
{
    glUniform4f(location, v0, v1, v2, v3);
    return *this;
}

OpenGLContext&
OpenGLContext::GLUniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    glUniformMatrix4fv(location, count, transpose, value);
    return *this;
}

GLint OpenGLContext::GLGetUniformLocation(GLuint program, const GLchar* name)
{
    return glGetUniformLocation(program, name);
}

OpenGLContext& OpenGLContext::GLClearColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
    glClearColor(red, green, blue, alpha);
    return *this;
}

OpenGLContext& OpenGLContext::GLClearDepth(GLfloat depth)
{
    glClearDepth(depth);
    return *this;
}

OpenGLContext& OpenGLContext::GLClear(GLbitfield mask)
{
    glClear(mask);
    return *this;
}

OpenGLContext& OpenGLContext::GLDrawArrays(GLenum mode, GLint first, GLsizei count)
{
    glDrawArrays(mode, first, count);
    return *this;
}

OpenGLContext& OpenGLContext::GLDrawElements(GLenum mode, GLsizei count, GLenum type, const void* indices)
{
    glDrawElements(mode, count, type, indices);
    return *this;
}

OpenGLContext& OpenGLContext::GLCheck()
{
    GLenum err;
    auto getErrorString = [](GLenum error) -> const char* {
        switch (error) {
        case GL_INVALID_ENUM:
            return "GL_INVALID_ENUM";
        case GL_INVALID_VALUE:
            return "GL_INVALID_VALUE";
        case GL_INVALID_OPERATION:
            return "GL_INVALID_OPERATION";
        case GL_STACK_OVERFLOW:
            return "GL_STACK_OVERFLOW";
        case GL_STACK_UNDERFLOW:
            return "GL_STACK_UNDERFLOW";
        case GL_OUT_OF_MEMORY:
            return "GL_OUT_OF_MEMORY";
#ifdef GL_INVALID_FRAMEBUFFER_OPERATION
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            return "GL_INVALID_FRAMEBUFFER_OPERATION";
#endif
#ifdef GL_CONTEXT_LOST
        case GL_CONTEXT_LOST:
            return "GL_CONTEXT_LOST";
#endif
        default:
            return "Unknown OpenGL Error";
        }
    };
    while ((err = glGetError()) != GL_NO_ERROR) {
        std::cerr << "OpenGL Error: " << getErrorString(err) << std::endl;
    }
    return *this;
}

} // namespace CS
