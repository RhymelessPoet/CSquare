#include "GraphicsGLImpl.h"
#include "GraphicsBufferDescriptor.h"
#include "GraphicsInputAssemblyDescriptor.h"
#include "GraphicsPipelineDescriptor.h"
#include "GraphicsResourceParameters.h"
#include "GraphicsShaderStage.h"
#include "ShaderBindingSetDescriptor.h"
#include "base/EnumUtils.h"
#include "graphics/GraphicsResourceCache.h"
#include "graphics/opengl/glad/include/glad/glad.h"
#include "opengl/OpenGLContext.h"
#include <iostream>

namespace CS
{
//  GL_BYTE, GL_UNSIGNED_BYTE, GL_SHORT, GL_UNSIGNED_SHORT, GL_INT, and GL_UNSIGNED_INT are accepted by
//  glVertexAttribPointer and glVertexAttribIPointer. Additionally GL_HALF_FLOAT, GL_FLOAT, GL_DOUBLE, GL_FIXED,
//  GL_INT_2_10_10_10_REV, GL_UNSIGNED_INT_2_10_10_10_REV and GL_UNSIGNED_INT_10F_11F_11F_REV
using AttrFormat = VertexInputFormat;
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

static inline GLenum GetIndexFormatType(IndexFormat format)
{
    if (IndexFormat::UInt16 == format) {
        return GL_UNSIGNED_SHORT;
    }
    if (IndexFormat::UInt32 == format) {
        return GL_UNSIGNED_INT;
    }
    return GL_UNSIGNED_INT;
}

static inline GLenum GetBufferType(GraphicsBufferDescriptor::BufferType type)
{
    switch (type) {
    case GraphicsBufferDescriptor::BufferType::VertexBuffer:
        return GL_ARRAY_BUFFER;
    case GraphicsBufferDescriptor::BufferType::IndexBuffer:
        return GL_ELEMENT_ARRAY_BUFFER;
    case GraphicsBufferDescriptor::BufferType::UniformBuffer:
        return GL_UNIFORM_BUFFER;
    case GraphicsBufferDescriptor::BufferType::StorageBuffer:
        return GL_SHADER_STORAGE_BUFFER;
    case GraphicsBufferDescriptor::BufferType::IndirectBuffer:
        return GL_DRAW_INDIRECT_BUFFER;
    default:
        return GL_ARRAY_BUFFER;
    }
}

static inline GLenum GetSamplerAddressMode(AddressMode mode)
{
    std::unordered_map<AddressMode, GLenum> map = {{AddressMode::Repeat, GL_REPEAT},
                                                   {AddressMode::MirroredRepeat, GL_MIRRORED_REPEAT},
                                                   {AddressMode::ClampToEdge, GL_CLAMP_TO_EDGE},
                                                   {AddressMode::ClampToBorder, GL_CLAMP_TO_BORDER}};

    return map[mode];
}

static inline GLenum GetSamplerMagFilterMode(FilterMode mode)
{
    std::unordered_map<FilterMode, GLenum> map = {{FilterMode::Linear, GL_LINEAR}, {FilterMode::Nearest, GL_NEAREST}};

    return map[mode];
}

static inline GLenum GetSamplerMinFilterMode(MipmapFilterMode mipmapMode, FilterMode mode)
{
    using GLEnumArray1D = std::array<GLenum, EnumValue(FilterMode::Max)>;
    std::array<GLEnumArray1D, EnumValue(MipmapFilterMode::Max) + 1u> map = {
        GLEnumArray1D{GL_NEAREST_MIPMAP_NEAREST, GL_NEAREST_MIPMAP_LINEAR},
        GLEnumArray1D{GL_LINEAR_MIPMAP_NEAREST, GL_LINEAR_MIPMAP_LINEAR}, GLEnumArray1D{GL_NEAREST, GL_LINEAR}};

    return map[EnumValue(mipmapMode)][EnumValue(mode)];
}

struct GLFormatMapping
{
    GLint internalFormat;
    GLenum format;
    GLenum type;
};

static GLFormatMapping GetGLFormatMapping(TextureFormat format)
{
    switch (format) {
    case TextureFormat::RGBA8Unorm:
        return {GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE};
    case TextureFormat::RGB8Unorm:
        return {GL_RGB8, GL_RGB, GL_UNSIGNED_BYTE};
    case TextureFormat::BGRA8Unorm:
        return {GL_RGBA8, GL_BGRA, GL_UNSIGNED_BYTE};
    case TextureFormat::RGBA8Srgb:
        return {GL_SRGB8_ALPHA8, GL_RGBA, GL_UNSIGNED_BYTE};
    case TextureFormat::RGB8Srgb:
        return {GL_SRGB8, GL_RGB, GL_UNSIGNED_BYTE};
    case TextureFormat::RGB32Float:
        return {GL_RGB32F, GL_RGB, GL_FLOAT};
    case TextureFormat::RGBA32Float:
        return {GL_RGBA32F, GL_RGBA, GL_FLOAT};
    case TextureFormat::Depth24Stencil8:
        return {GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8};
    case TextureFormat::Depth32:
        return {GL_DEPTH_COMPONENT32, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT};
    default:
        return {GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE};
    }
}

static GLenum GetGLCompareOp(DepthCompareOp op)
{
    switch (op) {
    case DepthCompareOp::Never:
        return GL_NEVER;
    case DepthCompareOp::Less:
        return GL_LESS;
    case DepthCompareOp::Equal:
        return GL_EQUAL;
    case DepthCompareOp::LessEqual:
        return GL_LEQUAL;
    case DepthCompareOp::Greater:
        return GL_GREATER;
    case DepthCompareOp::NotEqual:
        return GL_NOTEQUAL;
    case DepthCompareOp::GreaterEqual:
        return GL_GEQUAL;
    case DepthCompareOp::Always:
        return GL_ALWAYS;
    default:
        return GL_LESS;
    }
}

GraphicsGLImpl::GraphicsGLImpl(std::unique_ptr<OpenGLContext> context,
                               std::shared_ptr<GraphicsResourceCache> resourceCache)
    : m_glContext(std::move(context)), m_resouceCache(resourceCache)
{}

GraphicsGLImpl::~GraphicsGLImpl() {}

bool GraphicsGLImpl::Initialize()
{
    bool glLoad = gladLoadGL() == 1;

    std::cerr << m_glContext->GetVersion() << std::endl;

    return glLoad;
}

bool GraphicsGLImpl::SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
{
    m_glContext->GLViewport(x, y, width, height);
    return true;
}

bool GraphicsGLImpl::BuildGraphicsBuffer(GraphicsBufferDescriptor* descriptor)
{
    GLuint buffer = 0u;
    GLenum target = GetBufferType(descriptor->GetBufferType());
    m_glContext->GLGenBuffers(1, &buffer)
        .GLBindBuffer(target, buffer)
        .GLBufferData(target, descriptor->GetSize(), nullptr, GL_DYNAMIC_DRAW)
        .GLBindBuffer(target, 0)
        .GLCheck();
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

    GLenum target = GetBufferType(bufferType);
    if (target == GL_UNIFORM_BUFFER) {
        m_glContext->GLBindBuffer(target, buffer);
        m_glContext->GLCheck();
        // .GLBufferSubData(GL_UNIFORM_BUFFER, 0, size, data)
        m_glContext->GLBufferData(target, size, data, GL_DYNAMIC_DRAW);

        m_glContext->GLCheck();

        m_glContext->GLBindBuffer(target, 0);

        m_glContext->GLCheck();
    } else {
        m_glContext->GLBindBuffer(target, buffer)
            .GLBufferData(target, size, data, GL_STATIC_DRAW)
            .GLBindBuffer(target, 0);
    }
    return true;
}

bool GraphicsGLImpl::UpdateGraphicsSubBufferData(GraphicsBufferDescriptor* descriptor,
                                                 std::span<const std::byte> data,
                                                 size_t offset)
{
    GLuint buffer = descriptor->GetNativeBuffer();
    auto bufferType = descriptor->GetBufferType();

    auto bufferSize = descriptor->GetSize();
    auto dataSize = data.size();
    if (buffer == 0u || dataSize == 0u || offset + dataSize > bufferSize) {
        return false;
    }

    GLenum target = GetBufferType(bufferType);
    if (target == GL_UNIFORM_BUFFER) {
        m_glContext->GLBindBuffer(target, buffer);
        // .GLBufferSubData(GL_UNIFORM_BUFFER, 0, size, data)
        m_glContext->GLBufferSubData(target, offset, dataSize, data.data());

        m_glContext->GLCheck();

        m_glContext->GLBindBuffer(target, 0);

        m_glContext->GLCheck();
    } else {
        m_glContext->GLBindBuffer(target, buffer)
            .GLBufferSubData(target, offset, dataSize, data.data())
            .GLBindBuffer(target, 0);
    }
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

        const auto& [vertexBuffer, bindingOffset] = descriptor->GetVertexInput(bindingNum);

        auto [componentSize, componentCount] = attrValue.GetSize();
        auto format = GetVertexAttribFormat(attrValue.GetFormat());
        auto offset = attrValue.GetOffset() + bindingOffset;

        auto stride = binding.value().GetStride();

        if (vertexBuffer == nullptr) {
            continue;
        }
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
    bool hasDepthStencilAttachment = false;

    auto colorTexture = descriptor->GetColorAttachment();
    if (colorTexture != nullptr) {
        hasColorAttachment = colorTexture->Build();
    }
    auto depthTexture = descriptor->GetDepthAttachment();
    if (depthTexture != nullptr) {
        hasDepthAttachment = depthTexture->Build();
    }
    auto depthStencilTexture = descriptor->GetDepthStencilAttachment();
    if (depthStencilTexture != nullptr) {
        hasDepthStencilAttachment = depthStencilTexture->Build();
    }

    m_glContext->GLGenFramebuffers(1, &fbo).GLBindFramebuffer(GL_FRAMEBUFFER, fbo);

    if (hasColorAttachment) {
        m_glContext->GLFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                                            colorTexture->GetNativeTexture(), 0);
    }
    if (hasDepthStencilAttachment) {
        m_glContext->GLFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D,
                                            depthStencilTexture->GetNativeTexture(), 0);
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
        auto vertexSource = vertexStage->GetShader()->GetSource().data();
        m_glContext->GLCreateShader(GL_VERTEX_SHADER, &pipeline.vertexShaderID)
            .GLShaderSource(pipeline.vertexShaderID, 1, &vertexSource, nullptr)
            .GLCompileShader(pipeline.vertexShaderID);
    }

    if (fragmentStage != nullptr) {
        auto fragmentSource = fragmentStage->GetShader()->GetSource().data();
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

    const auto& [depthCompareOp, depthTest] = descriptor->GetDepthStencilState();
    if (depthTest) {
        m_glContext->GLEnable(GL_DEPTH_TEST);
    } else {
        m_glContext->GLDisable(GL_DEPTH_TEST);
    }
    m_glContext->GLDisable(GL_STENCIL_TEST);

    m_glContext->GLDepthFunc(GetGLCompareOp(depthCompareOp));

    m_curentStates.SetPipeline(descriptor);
    return true;
}

bool GraphicsGLImpl::BindShaderBindingSet(ShaderBindingSetDescriptor* descriptor)
{
    if (!descriptor->IsBuild()) {
        return false;
    }
    auto layout = descriptor->GetLayout();
    auto& bindings = layout->GetBindings();

    bool hasError = false;
    for (const auto& binding : bindings) {
        if (binding.GetType() == ShaderBinding::Type::UniformBuffer) {
            hasError = !bindUniformBuffer(descriptor, binding);
        } else if (binding.GetType() == ShaderBinding::Type::SampledTexture) {
            hasError = !bindSampledTexture(descriptor, binding);
        }
    }

    return !hasError;
}

bool GraphicsGLImpl::BuildTexture(TextureDescriptor* descriptor)
{
    GLuint textureID = 0u;
    const auto [width, height] = descriptor->GetSize();
    auto [internalFormat, format, type] = GetGLFormatMapping(descriptor->GetFormat());

    m_glContext->GLGenTextures(1, &textureID)
        .GLBindTexture(GL_TEXTURE_2D, textureID)
        .GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR)
        .GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR)
        .GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT)
        .GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT)
        .GLTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, type, nullptr)
        .GLBindTexture(GL_TEXTURE_2D, 0);

    descriptor->SetNativeTexture(textureID);
    return true;
}

bool GraphicsGLImpl::UpdateTextureData(TextureDescriptor* descriptor, const void* data)
{
    static constexpr GLint DefaultTextureDataAlignment = 4;

    GLuint textureID = descriptor->GetNativeTexture();
    const auto [width, height] = descriptor->GetSize();
    auto [internalFormat, format, type] = GetGLFormatMapping(descriptor->GetFormat());

    auto formatSize = GetTextureFormatSize(descriptor->GetFormat());
    auto alignment = (width * formatSize) % DefaultTextureDataAlignment;

    if (alignment != 0) {
        alignment = alignment % 2 == 0 ? 2 : 1;
        m_glContext->GLPixelStorei(GL_UNPACK_ALIGNMENT, static_cast<GLint>(alignment)).GLCheck();
    }
    m_glContext->GLBindTexture(GL_TEXTURE_2D, textureID)
        .GLTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, type, data)
        .GLCheck();
    if (descriptor->IsMipmap()) {
        m_glContext->GLGenerateMipmap(GL_TEXTURE_2D).GLCheck();
    }
    m_glContext->GLBindTexture(GL_TEXTURE_2D, 0);
    if (alignment != 0) {
        m_glContext->GLPixelStorei(GL_UNPACK_ALIGNMENT, DefaultTextureDataAlignment);
    }

    return true;
}

bool GraphicsGLImpl::DestroyTexture(TextureDescriptor* descriptor)
{
    GLuint textureID = descriptor->GetNativeTexture();
    m_glContext->GLBindTexture(GL_TEXTURE_2D, 0).GLDeleteTextures(1, &textureID).GLCheck();
    descriptor->SetNativeTexture(0);

    return true;
}

bool GraphicsGLImpl::BuildSampler(SamplerDescriptor* descriptor)
{
    GLuint samplerID = 0u;

    auto [u, v] = descriptor->GetAddressModeUV();
    auto w = descriptor->GetAddressModeW();
    auto [min, mag] = descriptor->GetFilterMode();
    auto mipmapFilter = descriptor->GetMipmapFilter();

    m_glContext->GLGenSamplers(1, &samplerID)
        .GLSamplerParameteri(samplerID, GL_TEXTURE_WRAP_S, GetSamplerAddressMode(u))
        .GLSamplerParameteri(samplerID, GL_TEXTURE_WRAP_T, GetSamplerAddressMode(v))
        .GLSamplerParameteri(samplerID, GL_TEXTURE_WRAP_R, GetSamplerAddressMode(w))
        .GLSamplerParameteri(samplerID, GL_TEXTURE_MIN_FILTER, GetSamplerMinFilterMode(mipmapFilter, min))
        .GLSamplerParameteri(samplerID, GL_TEXTURE_MAG_FILTER, GetSamplerMagFilterMode(mag));

    descriptor->SetNativeSampler(samplerID);

    return true;
}

bool GraphicsGLImpl::DestroySampler(SamplerDescriptor* descriptor)
{
    auto samplerID = descriptor->GetNativeSampler();
    m_glContext->GLDeleteSamplers(1, &samplerID);
    descriptor->SetNativeSampler(0);

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
        m_glContext->GLDepthMask(GL_TRUE);
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

bool GraphicsGLImpl::ResetCurrentState()
{
    m_curentStates.Reset();
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

bool GraphicsGLImpl::bindUniformBuffer(ShaderBindingSetDescriptor* descriptor, const ShaderBinding& binding)
{
    auto bindingNum = binding.GetBinding();
    auto bindingInfo = descriptor->GetBindingInfo<ShaderBindingSetDescriptor::UniformBufferBinding>(bindingNum);

    if (!bindingInfo.has_value()) {
        return false;
    }

    auto [uniformBuffer, offset, range] = bindingInfo.value();
    if (uniformBuffer != nullptr) {
        auto buffer = uniformBuffer->GetNativeBuffer();
        m_glContext->GLBindBufferRange(GL_UNIFORM_BUFFER, bindingNum, buffer, offset, range);
        m_glContext->GLCheck();
        return true;
    }

    return false;
}

bool GraphicsGLImpl::bindSampledTexture(ShaderBindingSetDescriptor* descriptor, const ShaderBinding& binding)
{
    auto bindingNum = binding.GetBinding();
    auto bindingInfo = descriptor->GetBindingInfo<ShaderBindingSetDescriptor::SampledTextureBinding>(bindingNum);

    if (!bindingInfo.has_value()) {
        return false;
    }

    auto [texture, sampler] = bindingInfo.value();
    if (texture != nullptr && sampler != nullptr) {
        // clang-format off
        m_glContext->GLActiveTexture(GL_TEXTURE0 + bindingNum)
                    .GLBindTexture(GL_TEXTURE_2D, texture->GetNativeTexture())
                    .GLBindSampler(bindingNum, sampler->GetNativeSampler());
        // clang-format on
        return true;
    }

    return false;
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
