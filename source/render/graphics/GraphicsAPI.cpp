#include "GraphicsAPI.h"
#include "GraphicsBufferDescriptor.h"
#include "GraphicsCommandBufferDescriptor.h"
#include "GraphicsInputAssemblyDescriptor.h"
#include "GraphicsPipelineDescriptor.h"
#include "GraphicsResourceCache.h"
#include "GraphicsResourceDescriptors.h"
#include "opengl/GraphicsGLImpl.h"
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

VertexBuffer GraphicsAPI::CreateVertexBuffer(size_t size)
{
    auto resorceCache = m_impl->GetResourceCache();

    auto resourceID = resorceCache->Allocate<GraphicsBufferDescriptor>(m_impl);
    auto descriptor = resorceCache->GetDescriptor<GraphicsBufferDescriptor>(resourceID);
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
    return UniformBuffer();
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

Texture GraphicsAPI::CreateTexture()
{
    auto resorceCache = m_impl->GetResourceCache();

    auto resourceID = resorceCache->Allocate<TextureDescriptor>(m_impl);
    auto descriptor = resorceCache->GetDescriptor<TextureDescriptor>(resourceID);

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

Texture GraphicsAPI::GetColorAttachment(RenderTarget renderTarget) const
{
    return GetTexture(renderTarget.GetColorAttachment());
}

RenderTarget GraphicsAPI::CreateRenderTarget(const Size2U& size)
{
    auto resorceCache = m_impl->GetResourceCache();

    auto resourceID = resorceCache->Allocate<RenderTargetDescriptor>(m_impl);
    auto descriptor = resorceCache->GetDescriptor<RenderTargetDescriptor>(resourceID);

    return RenderTarget(descriptor);
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
