#include "OpenGLContext.h"
#include "NativeContext.h"
#include <format>
#include <iostream>

namespace CS
{

OpenGLContext::OpenGLContext()
{
    m_nativeContext = std::make_unique<GLFWContext>();
}

OpenGLContext::OpenGLContext(std::shared_ptr<OpenGLContext> sharedContext) : m_sharedContext(std::move(sharedContext))
{}

OpenGLContext::OpenGLContext(std::unique_ptr<INativeContext> nativeContext) : m_nativeContext(std::move(nativeContext))
{}

OpenGLContext::~OpenGLContext() {}

bool OpenGLContext::IsShared() const
{
    return m_sharedContext != nullptr;
}

bool OpenGLContext::MakeCurrent()
{
    if (m_nativeContext)
        return m_nativeContext->MakeCurrent();
    return m_sharedContext ? m_sharedContext->MakeCurrent() : false;
}

void OpenGLContext::DoneCurrent()
{
    if (m_nativeContext)
        m_nativeContext->DoneCurrent();
    else if (m_sharedContext)
        m_sharedContext->DoneCurrent();
}

void OpenGLContext::SwapBuffers()
{
    if (m_nativeContext)
        m_nativeContext->SwapBuffers();
    else if (m_sharedContext)
        m_sharedContext->SwapBuffers();
}

std::string_view OpenGLContext::GetVersion()
{
    if (m_version.empty()) {
        initVersion();
    }
    return m_version;
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

OpenGLContext& OpenGLContext::GLCullFace(GLenum mode)
{
    glCullFace(mode);
    return *this;
}

OpenGLContext& OpenGLContext::GLBlendFuncSeparate(GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha)
{
    glBlendFuncSeparate(srcRGB, dstRGB, srcAlpha, dstAlpha);
    return *this;
}

OpenGLContext& OpenGLContext::GLBlendEquationSeparate(GLenum modeRGB, GLenum modeAlpha)
{
    glBlendEquationSeparate(modeRGB, modeAlpha);
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

OpenGLContext& OpenGLContext::GLBlitFramebuffer(GLint srcX0,
                                                GLint srcY0,
                                                GLint srcX1,
                                                GLint srcY1,
                                                GLint dstX0,
                                                GLint dstY0,
                                                GLint dstX1,
                                                GLint dstY1,
                                                GLbitfield mask,
                                                GLenum filter)
{
    glBlitFramebuffer(srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1, mask, filter);
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

OpenGLContext& OpenGLContext::GLTexSubImage2D(GLenum target,
                                              GLint level,
                                              GLint xoffset,
                                              GLint yoffset,
                                              GLsizei width,
                                              GLsizei height,
                                              GLenum format,
                                              GLenum type,
                                              const void* pixels)
{
    glTexSubImage2D(target, level, xoffset, yoffset, width, height, format, type, pixels);
    return *this;
}

OpenGLContext& OpenGLContext::GLDeleteTextures(GLsizei n, const GLuint* textures)
{
    glDeleteTextures(n, textures);
    return *this;
}

OpenGLContext& OpenGLContext::GLPixelStorei(GLenum pname, GLint param)
{
    glPixelStorei(pname, param);
    return *this;
}

OpenGLContext& OpenGLContext::GLGenerateMipmap(GLenum target)
{
    glGenerateMipmap(target);
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
    if (target == GL_UNIFORM_BUFFER && offset % alignment != 0) {
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

OpenGLContext& OpenGLContext::GLDeleteProgram(GLuint program)
{
    glDeleteProgram(program);
    return *this;
}

OpenGLContext& OpenGLContext::GLGetBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, void* data)
{
    glGetBufferSubData(target, offset, size, data);
    return *this;
}

OpenGLContext& OpenGLContext::GLDispatchCompute(GLuint numGroupsX, GLuint numGroupsY, GLuint numGroupsZ)
{
    glDispatchCompute(numGroupsX, numGroupsY, numGroupsZ);
    return *this;
}

OpenGLContext& OpenGLContext::GLMemoryBarrier(GLbitfield barriers)
{
    glMemoryBarrier(barriers);
    return *this;
}

OpenGLContext& OpenGLContext::GLBindImageTexture(GLuint unit,
                                                 GLuint texture,
                                                 GLint level,
                                                 GLboolean layered,
                                                 GLint layer,
                                                 GLenum access,
                                                 GLenum format)
{
    glBindImageTexture(unit, texture, level, layered, layer, access, format);
    return *this;
}

OpenGLContext& OpenGLContext::GLGetTexImage(GLenum target, GLint level, GLenum format, GLenum type, void* pixels)
{
    glGetTexImage(target, level, format, type, pixels);
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

OpenGLContext& OpenGLContext::GLSetupDebugMessageCallback(GLDEBUGPROC callback, const void* userParam)
{
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(callback, userParam);

    return *this;
}

void OpenGLContext::initVersion()
{
    const char* version = reinterpret_cast<const char*>(glGetString(GL_VERSION));
    const char* vendor = reinterpret_cast<const char*>(glGetString(GL_VENDOR));
    const char* renderer = reinterpret_cast<const char*>(glGetString(GL_RENDERER));
    const char* glslVersion = reinterpret_cast<const char*>(glGetString(GL_SHADING_LANGUAGE_VERSION));

    m_version = std::format("[{}], [{}], [{}], [{}]", version, vendor, renderer, glslVersion);
}

} // namespace CS
