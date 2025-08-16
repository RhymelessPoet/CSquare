#include "GraphicsGLImpl.h"
#include "GraphicsBufferDescriptor.h"
#include "GraphicsInputAssemblyDescriptor.h"
#include "GraphicsPipelineDescriptor.h"
#include "GraphicsShaderStage.h"
#include "OpenGLContext.h"
#include "graphics/GraphicsResourceCache.h"
#include "graphics/opengl/glad/include/glad/glad.h"

namespace CS
{
//  GL_BYTE, GL_UNSIGNED_BYTE, GL_SHORT, GL_UNSIGNED_SHORT, GL_INT, and GL_UNSIGNED_INT are accepted by
//  glVertexAttribPointer and glVertexAttribIPointer. Additionally GL_HALF_FLOAT, GL_FLOAT, GL_DOUBLE, GL_FIXED,
//  GL_INT_2_10_10_10_REV, GL_UNSIGNED_INT_2_10_10_10_REV and GL_UNSIGNED_INT_10F_11F_11F_REV
using AttrFormat = VertexInputAttribute::Format;
static GLenum GetVertexAttribFormat(AttrFormat format)
{
    if (format == AttrFormat::Float || format == AttrFormat::Float2 || format == AttrFormat::Float3 ||
        format == AttrFormat::Float4)
    {
        return GL_FLOAT;
    } else if (format == AttrFormat::Int || format == AttrFormat::Int2 || format == AttrFormat::Int3 ||
               format == AttrFormat::Int4)
    {
        return GL_INT;
    } else if (format == AttrFormat::UInt || format == AttrFormat::UInt2 || format == AttrFormat::UInt3 ||
               format == AttrFormat::UInt4)
    {
        return GL_UNSIGNED_INT;
    }
    return GL_BYTE;
}

static GLenum GetIndexFormatType(IndexFormat format)
{
    if (IndexFormat::UInt16 == format) {
        return GL_UNSIGNED_SHORT;
    }
    if (IndexFormat::UInt32 == format) {
        return GL_UNSIGNED_INT;
    }
    return GL_UNSIGNED_INT;
}

GraphicsGLImpl::GraphicsGLImpl(std::unique_ptr<OpenGLContext> context,
                               std::shared_ptr<GraphicsResourceCache> resourceCache)
    : m_glContext(std::move(context)), m_resouceCache(resourceCache)
{}

GraphicsGLImpl::~GraphicsGLImpl() {}

bool GraphicsGLImpl::Initialize()
{
    return gladLoadGL() == 1;
}

bool GraphicsGLImpl::SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
{
    m_glContext->GLViewport(x, y, width, height);
    return true;
}

bool GraphicsGLImpl::BuildGraphicsBuffer(GraphicsBufferDescriptor* descriptor)
{
    GLuint buffer = 0u;
    m_glContext->GLGenBuffers(1, &buffer);
    descriptor->SetNativeBuffer(buffer);
    return true;
}

bool GraphicsGLImpl::UpdateGraphicsBufferData(GraphicsBufferDescriptor* descriptor, const void* data)
{
    GLuint buffer = descriptor->GetNativeBuffer();
    auto bufferType = descriptor->GetBufferType();

    auto size = descriptor->GetSize();
    if (buffer == 0u || data == nullptr || size == 0u) {
        return false;
    }

    GLenum target =
        (bufferType == GraphicsBufferDescriptor::BufferType::VertexBuffer) ? GL_ARRAY_BUFFER : GL_ELEMENT_ARRAY_BUFFER;

    m_glContext->GLBindBuffer(target, buffer).GLBufferData(target, size, data, GL_STATIC_DRAW).GLBindBuffer(target, 0);

    return true;
}

bool GraphicsGLImpl::DestroyGraphicsBuffer(GraphicsBufferDescriptor* descriptor)
{
    GLuint buffer = descriptor->GetNativeBuffer();
    if (buffer != 0u) {
        m_glContext->GLBindBuffer(GL_ARRAY_BUFFER, 0).GLDeleteBuffers(1, &buffer);
        descriptor->SetNativeBuffer(0);
        return true;
    }
    return false;
}

bool GraphicsGLImpl::IsBuild(const GraphicsInputAssemblyDescriptor* descriptor)
{
    const auto vao = descriptor->GetAssemblyInputData<GLuint>();
    return vao != 0u;
}

bool GraphicsGLImpl::BuildGraphicsInputAssembly(GraphicsInputAssemblyDescriptor* descriptor)
{
    GLuint vao = 0u;
    m_glContext->GLGenVertexArrays(1, &vao);
    const auto& vertexInputLayout = descriptor->GetVertexInputLayout();

    m_glContext->GLBindVertexArray(vao);

    for (uint32_t location = 0u; location < VertexInputLayout::MaxAttributes; ++location) {
        const auto& attribute = vertexInputLayout.GetAttribute(location);
        if (!attribute.has_value()) {
            continue;
        }
        const auto& attrValue = attribute.value();
        auto bindingNum = attrValue.GetBinding();
        const auto& binding = vertexInputLayout.GetBinding(bindingNum);

        if (!binding.has_value()) {
            continue;
        }

        const auto& [vertexBuffer, _] = descriptor->GetVertexInput(bindingNum);

        auto [componentSize, componentCount] = attrValue.GetSize();
        auto format = GetVertexAttribFormat(attrValue.GetFormat());
        auto offset = attrValue.GetOffset();

        auto stride = binding.value().GetStride();

        m_glContext->GLBindBuffer(GL_ARRAY_BUFFER, vertexBuffer->GetNativeBuffer())
            .GLVertexAttribPointer(location, componentCount, format, false, stride,
                                   reinterpret_cast<void*>(static_cast<intptr_t>(offset)))
            .GLEnableVertexAttribArray(location);
    }
    m_glContext->GLBindBuffer(GL_ELEMENT_ARRAY_BUFFER, descriptor->GetIndexBuffer()->GetNativeBuffer());
    m_glContext->GLBindVertexArray(0);
    m_glContext->GLBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    m_glContext->GLBindBuffer(GL_ARRAY_BUFFER, 0);

    descriptor->SetAssemblyInputData(vao);
    return true;
}

bool GraphicsGLImpl::BindGraphicsInputAssembly(GraphicsInputAssemblyDescriptor* descriptor)
{
    auto vao = descriptor->GetAssemblyInputData<GLuint>();
    m_glContext->GLBindVertexArray(vao);
    m_curentStates.SetInputAssembly(descriptor);
    return true;
}

bool GraphicsGLImpl::DestroyGraphicsInputAssembly(GraphicsInputAssemblyDescriptor* descriptor)
{
    auto vao = descriptor->GetAssemblyInputData<GLuint>();
    if (vao != 0u) {
        m_glContext->GLBindVertexArray(0).GLDeleteVertexArrays(1, &vao);
        descriptor->SetAssemblyInputData(0u);
        return true;
    }
    return false;
}

bool GraphicsGLImpl::BindRenderTarget(RenderTargetDescriptor* descriptor)
{
    auto nativeFBO = descriptor->GetNativeFBO();
    if (nativeFBO.has_value()) {
        m_glContext->GLBindFramebuffer(GL_FRAMEBUFFER, nativeFBO.value());
        return true;
    }
    return false;
}

bool GraphicsGLImpl::BuildRenderTarget(RenderTargetDescriptor* descriptor)
{
    GLuint fbo = 0u;

    bool hasColorAttachment = false;
    bool hasDepthAttachment = false;

    auto colorTexture = descriptor->GetColorAttachment();
    if (colorTexture != nullptr) {
        hasColorAttachment = colorTexture->Build();
    }
    auto depthTexture = descriptor->GetDepthAttachment();
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

bool GraphicsGLImpl::DestroyRenderTarget(RenderTargetDescriptor* descriptor)
{
    auto nativeFBO = descriptor->GetNativeFBO();
    if (nativeFBO.has_value()) {
        m_glContext->GLBindFramebuffer(GL_FRAMEBUFFER, 0).GLDeleteFramebuffers(1, &nativeFBO.value());
        descriptor->SetNativeFBO(0);
    }
    return true;
}

struct GLGraphicsPipeline
{
    GLuint programID{0u};
    GLuint vertexShaderID{0u};
    GLuint fragmentShaderID{0u};
};

bool GraphicsGLImpl::IsBuild(const GraphicsPipelineDescriptor* descriptor)
{
    const auto& pipeline = descriptor->GetNativePipelineData<GLGraphicsPipeline>();
    return pipeline.programID != 0u;
}

bool GraphicsGLImpl::BuildGraphicsPipeline(GraphicsPipelineDescriptor* descriptor)
{
    GLGraphicsPipeline pipeline;
    const auto& vertexStage = descriptor->GetShaderStage(ShaderStage::Vertex);
    const auto& fragmentStage = descriptor->GetShaderStage(ShaderStage::Fragment);

    if (vertexStage != nullptr) {
        auto vertexSource = vertexStage->GetShader()->GetSource().c_str();
        m_glContext->GLCreateShader(GL_VERTEX_SHADER, &pipeline.vertexShaderID)
            .GLShaderSource(pipeline.vertexShaderID, 1, &vertexSource, nullptr)
            .GLCompileShader(pipeline.vertexShaderID);
    }

    if (fragmentStage != nullptr) {
        auto fragmentSource = fragmentStage->GetShader()->GetSource().c_str();
        m_glContext->GLCreateShader(GL_FRAGMENT_SHADER, &pipeline.fragmentShaderID)
            .GLShaderSource(pipeline.fragmentShaderID, 1, &fragmentSource, nullptr)
            .GLCompileShader(pipeline.fragmentShaderID);
    }

    m_glContext->GLCreateProgram(&pipeline.programID);
    if (pipeline.vertexShaderID != 0u) {
        m_glContext->GLAttachShader(pipeline.programID, pipeline.vertexShaderID);
    }
    if (pipeline.fragmentShaderID != 0u) {
        m_glContext->GLAttachShader(pipeline.programID, pipeline.fragmentShaderID);
    }
    m_glContext->GLLinkProgram(pipeline.programID);

    GLint linkStatus;
    m_glContext->GLGetProgramiv(pipeline.programID, GL_LINK_STATUS, &linkStatus);

    if (linkStatus == GL_TRUE) {
        descriptor->SetNativePipelineData(std::move(pipeline));
        return true;
    }

    return false;
}

bool GraphicsGLImpl::BindGraphicsPipeline(GraphicsPipelineDescriptor* descriptor)
{
    if (!descriptor->IsBuild()) {
        return false;
    }
    const auto& pipeline = descriptor->GetNativePipelineData<GLGraphicsPipeline>();

    m_glContext->GLUseProgram(pipeline.programID);
    m_glContext->GLDisable(GL_DEPTH_TEST);
    m_glContext->GLDisable(GL_STENCIL_TEST);

    m_curentStates.SetPipeline(descriptor);
    return true;
}

bool GraphicsGLImpl::BuildTexture(TextureDescriptor* descriptor)
{
    GLuint textureID = 0u;
    auto textureSize = descriptor->GetSize();

    m_glContext->GLGenTextures(1, &textureID)
        .GLBindTexture(GL_TEXTURE_2D, textureID)
        .GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR)
        .GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR)
        .GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT)
        .GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT)
        .GLTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, textureSize.Width(), textureSize.Height(), 0, GL_RGBA,
                      GL_UNSIGNED_BYTE, descriptor->GetData())
        .GLBindTexture(GL_TEXTURE_2D, 0);

    descriptor->SetNativeTexture(textureID);
    return true;
}

bool GraphicsGLImpl::UpdateTextureData(TextureDescriptor* descriptor)
{
    GLuint textureID = descriptor->GetNativeTexture();
    auto textureSize = descriptor->GetSize();

    m_glContext->GLBindTexture(GL_TEXTURE_2D, textureID)
        .GLTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, textureSize.Width(), textureSize.Height(), 0, GL_RGBA,
                      GL_UNSIGNED_BYTE, descriptor->GetData())
        .GLBindTexture(GL_TEXTURE_2D, 0);

    return true;
}

bool GraphicsGLImpl::DestroyTexture(TextureDescriptor* descriptor)
{
    GLuint textureID = descriptor->GetNativeTexture();
    m_glContext->GLBindTexture(GL_TEXTURE_2D, 0).GLDeleteTextures(1, &textureID);
    descriptor->SetNativeTexture(0);

    return true;
}

bool GraphicsGLImpl::Clear(std::optional<Color> color, std::optional<float> depth)
{
    GLbitfield mask = 0;
    if (color.has_value()) {
        m_glContext->GLClearColor(color->Red(), color->Green(), color->Blue(), color->Alpha());
        mask |= GL_COLOR_BUFFER_BIT;
    }
    if (depth.has_value()) {
        m_glContext->GLClearDepth(depth.value());
        mask |= GL_DEPTH_BUFFER_BIT;
    }
    if (mask != 0) {
        m_glContext->GLClear(mask);
    }
    return true;
}

bool GraphicsGLImpl::DrawIndexed(uint32_t indexCount,
                                 uint32_t instanceCount,
                                 uint32_t firstIndex,
                                 int32_t vertexOffset,
                                 uint32_t firstInstance)
{
    auto format = m_curentStates.GetInputAssembly()->GetIndexFormat();
    auto glFormat = GetIndexFormatType(format);

    m_glContext->GLDrawElements(GL_TRIANGLES, indexCount, glFormat,
                                reinterpret_cast<void*>(static_cast<intptr_t>(firstIndex)));
    m_glContext->GLBindVertexArray(0);
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

GraphicsGLImpl::CurrentStates::~CurrentStates()
{
    Reset();
}

void GraphicsGLImpl::CurrentStates::SetPipeline(GraphicsPipelineDescriptor* pipeline)
{
    if (m_pipeline != nullptr) {
        m_pipeline->Release();
    }

    m_pipeline = pipeline;

    if (m_pipeline != nullptr) {
        m_pipeline->AddReference();
    }
}

void GraphicsGLImpl::CurrentStates::SetInputAssembly(GraphicsInputAssemblyDescriptor* inputAssembly)
{
    if (m_inputAssembly != nullptr) {
        m_inputAssembly->Release();
    }

    m_inputAssembly = inputAssembly;

    if (m_inputAssembly != nullptr) {
        m_inputAssembly->AddReference();
    }
}

void GraphicsGLImpl::CurrentStates::Reset()
{
    if (m_pipeline != nullptr) {
        m_pipeline->Release();
        m_pipeline = nullptr;
    }

    if (m_inputAssembly != nullptr) {
        m_inputAssembly->Release();
        m_inputAssembly = nullptr;
    }
}

} // namespace CS
