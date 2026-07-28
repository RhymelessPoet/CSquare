#include "GraphicsAPI.h"
#include "ComputePipelineDescriptor.h"
#include "GraphicsBufferDescriptor.h"
#include "GraphicsCommandBufferDescriptor.h"
#include "GraphicsGLImpl.h"
#include "GraphicsInputAssemblyDescriptor.h"
#include "GraphicsPipelineDescriptor.h"
#include "GraphicsResourceCache.h"
#include "ShaderBindingSetDescriptor.h"
#include <stdexcept>

namespace CS
{

std::shared_ptr<GraphicsAPI> GraphicsAPI::Create(std::unique_ptr<GraphicsGLImpl> impl,
                                                 std::shared_ptr<GraphicsResourceCache> resourceCache)
{
    auto api = new GraphicsAPI(std::move(impl), std::move(resourceCache));
    return std::shared_ptr<GraphicsAPI>(api);
}

GraphicsAPI::GraphicsAPI(std::unique_ptr<GraphicsGLImpl> impl, std::shared_ptr<GraphicsResourceCache> resourceCache)
    : m_impl(std::move(impl))
{}

void GraphicsAPI::Initialize()
{
    if (!m_impl->Initialize()) {
        throw std::runtime_error("Graphics API fails to initialize");
    }
}

bool GraphicsAPI::MakeContextCurrent()
{
    return m_impl->MakeContextCurrent();
}

void GraphicsAPI::DoneContextCurrent()
{
    m_impl->DoneContextCurrent();
}

void GraphicsAPI::Present(RenderTarget source, const Size2u& windowSize)
{
    auto& rtDescriptor = source.descriptor<RenderTargetDescriptor>();
    auto nativeFBO = rtDescriptor.GetNativeFBO();
    if (!nativeFBO.has_value()) {
        return;
    }
    m_impl->BlitToScreen(nativeFBO.value(), rtDescriptor.GetSize(), windowSize);
    m_impl->SwapBuffers();
}

void GraphicsAPI::SwapBuffers()
{
    m_impl->SwapBuffers();
}

VertexBuffer GraphicsAPI::CreateVertexBuffer(size_t size)
{
    auto resourceCache = m_impl->GetResourceCache();

    auto resourceID = resourceCache->Allocate<GraphicsBufferDescriptor>(m_impl);
    auto descriptor = resourceCache->GetDescriptor<GraphicsBufferDescriptor>(resourceID);
    descriptor->SetBufferType(GraphicsBufferDescriptor::BufferType::VertexBuffer);
    descriptor->SetSize(size);

    return VertexBuffer(descriptor);
}

IndexBuffer GraphicsAPI::CreateIndexBuffer(size_t size)
{
    auto resorceCache = m_impl->GetResourceCache();

    auto resourceID = resorceCache->Allocate<GraphicsBufferDescriptor>(m_impl);
    auto descriptor = resorceCache->GetDescriptor<GraphicsBufferDescriptor>(resourceID);
    descriptor->SetBufferType(GraphicsBufferDescriptor::BufferType::IndexBuffer);
    descriptor->SetSize(size);

    return IndexBuffer(descriptor);
}

UniformBuffer GraphicsAPI::CreateUniformBuffer(size_t size)
{
    auto resorceCache = m_impl->GetResourceCache();

    auto resourceID = resorceCache->Allocate<GraphicsBufferDescriptor>(m_impl);
    auto descriptor = resorceCache->GetDescriptor<GraphicsBufferDescriptor>(resourceID);
    descriptor->SetBufferType(GraphicsBufferDescriptor::BufferType::UniformBuffer);

    descriptor->SetSize(size);

    return UniformBuffer(descriptor);
}

GraphicsInputAssembly GraphicsAPI::CreateInputAssembly()
{
    auto resorceCache = m_impl->GetResourceCache();

    auto resourceID = resorceCache->Allocate<GraphicsInputAssemblyDescriptor>(m_impl);
    auto descriptor = resorceCache->GetDescriptor<GraphicsInputAssemblyDescriptor>(resourceID);

    return GraphicsInputAssembly(descriptor);
}

GraphicsPipeline GraphicsAPI::CreatePipeline()
{
    auto resorceCache = m_impl->GetResourceCache();

    auto resourceID = resorceCache->Allocate<GraphicsPipelineDescriptor>(m_impl);
    auto descriptor = resorceCache->GetDescriptor<GraphicsPipelineDescriptor>(resourceID);

    return GraphicsPipeline(descriptor);
}

StorageBuffer GraphicsAPI::CreateStorageBuffer(size_t size)
{
    auto cache = m_impl->GetResourceCache();
    auto id = cache->Allocate<GraphicsBufferDescriptor>(m_impl);
    auto descriptor = cache->GetDescriptor<GraphicsBufferDescriptor>(id);
    descriptor->SetBufferType(GraphicsBufferDescriptor::BufferType::StorageBuffer);
    descriptor->SetSize(size);
    return StorageBuffer(descriptor);
}

VertexBuffer GraphicsAPI::CreateVertexBufferView(StorageBuffer buffer)
{
    return VertexBuffer(m_impl->GetResourceDescriptor<GraphicsBufferDescriptor>(buffer.GetID()));
}

std::vector<std::byte> GraphicsAPI::ReadStorageBuffer(StorageBuffer buffer, size_t offset, size_t size)
{
    auto descriptor = m_impl->GetResourceDescriptor<GraphicsBufferDescriptor>(buffer.GetID());
    return descriptor != nullptr && descriptor->Build() ? m_impl->ReadGraphicsBuffer(descriptor, offset, size)
                                                        : std::vector<std::byte>{};
}

ComputePipeline GraphicsAPI::CreateComputePipeline()
{
    auto resourceCache = m_impl->GetResourceCache();
    auto resourceID = resourceCache->Allocate<ComputePipelineDescriptor>(m_impl);
    return ComputePipeline(resourceCache->GetDescriptor<ComputePipelineDescriptor>(resourceID));
}

Texture GraphicsAPI::CreateTexture(TextureFormat format, const Size2u& size)
{
    auto resorceCache = m_impl->GetResourceCache();

    auto resourceID = resorceCache->Allocate<TextureDescriptor>(m_impl);
    auto descriptor = resorceCache->GetDescriptor<TextureDescriptor>(resourceID);

    descriptor->SetFormat(format);
    descriptor->SetSize(size);

    return Texture(descriptor);
}

Texture GraphicsAPI::GetTexture(size_t id) const
{
    auto resorceCache = m_impl->GetResourceCache();

    auto descriptor = resorceCache->GetDescriptor<TextureDescriptor>(id);
    if (!descriptor) {
        return Texture(nullptr);
    }

    return Texture(descriptor);
}

std::vector<std::byte> GraphicsAPI::ReadTexture(Texture texture)
{
    auto descriptor = m_impl->GetResourceDescriptor<TextureDescriptor>(texture.GetID());
    return descriptor != nullptr && descriptor->Build() ? m_impl->ReadTexture(descriptor) : std::vector<std::byte>{};
}

Texture GraphicsAPI::GetColorAttachment(RenderTarget renderTarget) const
{
    return GetTexture(renderTarget.GetColorAttachment());
}

Texture GraphicsAPI::GetDepthAttachment(RenderTarget renderTarget) const
{
    return GetTexture(renderTarget.GetDepthAttachment());
}

Texture GraphicsAPI::GetDepthStencilAttachment(RenderTarget renderTarget) const
{
    return GetTexture(renderTarget.GetDepthStencilAttachment());
}

Sampler GraphicsAPI::CreateSampler()
{
    auto resorceCache = m_impl->GetResourceCache();

    auto resourceID = resorceCache->Allocate<SamplerDescriptor>(m_impl);
    auto descriptor = resorceCache->GetDescriptor<SamplerDescriptor>(resourceID);

    return Sampler(descriptor);
}

RenderTarget GraphicsAPI::CreateRenderTarget(const Size2u& size)
{
    auto resorceCache = m_impl->GetResourceCache();

    auto resourceID = resorceCache->Allocate<RenderTargetDescriptor>(m_impl);
    auto descriptor = resorceCache->GetDescriptor<RenderTargetDescriptor>(resourceID);

    descriptor->SetSize(size);

    return RenderTarget(descriptor);
}

ShaderBindingSetLayout GraphicsAPI::CreateShaderBindingSetLayout()
{
    auto resorceCache = m_impl->GetResourceCache();

    auto resourceID = resorceCache->Allocate<ShaderBindingSetLayoutDescriptor>(m_impl);
    auto descriptor = resorceCache->GetDescriptor<ShaderBindingSetLayoutDescriptor>(resourceID);

    return ShaderBindingSetLayout(descriptor);
}

ShaderBindingSet GraphicsAPI::CreateShaderBindingSet(ShaderBindingSetLayout layout)
{
    auto resorceCache = m_impl->GetResourceCache();

    auto resourceID = resorceCache->Allocate<ShaderBindingSetDescriptor>(m_impl, layout.GetID());
    auto descriptor = resorceCache->GetDescriptor<ShaderBindingSetDescriptor>(resourceID);

    return ShaderBindingSet(descriptor);
}

GraphicsCommandBuffer GraphicsAPI::CreateCommandBuffer()
{
    auto resorceCache = m_impl->GetResourceCache();

    auto resourceID = resorceCache->Allocate<GraphicsCommandBufferDescriptor>(m_impl);
    auto descriptor = resorceCache->GetDescriptor<GraphicsCommandBufferDescriptor>(resourceID);

    return GraphicsCommandBuffer(descriptor);
}

void GraphicsAPI::SubmitCommandBuffer(GraphicsCommandBuffer commandBuffer)
{
    auto& descriptor = commandBuffer.descriptor<GraphicsCommandBufferDescriptor>();
    if (!descriptor.IsBuild()) {
        if (!descriptor.Build()) {
            throw std::runtime_error("Failed to build command buffer");
        }
    }

    descriptor.Execute(m_impl);
}

} // namespace CS
