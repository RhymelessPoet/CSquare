#include "GraphicsAPI.h"
#include "GraphicsCommandBufferDescriptor.h"
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

Texture GraphicsAPI::CreateTexture()
{
    auto resourceID = m_impl->GetResourceCache()->Allocate<TextureDescriptor>(m_impl);
    auto descriptor = m_impl->GetResourceCache()->GetDescriptor<TextureDescriptor>(resourceID);
    return Texture(descriptor);
}

Texture GraphicsAPI::GetTexture(size_t id) const
{
    auto descriptor = m_impl->GetResourceCache()->GetDescriptor<TextureDescriptor>(id);
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
    auto resourceID = m_impl->GetResourceCache()->Allocate<RenderTargetDescriptor>(m_impl);
    auto descriptor = m_impl->GetResourceCache()->GetDescriptor<RenderTargetDescriptor>(resourceID);
    return RenderTarget(descriptor);
}

GraphicsCommandBuffer GraphicsAPI::CreateCommandBuffer()
{
    auto resourceID = m_impl->GetResourceCache()->Allocate<GraphicsCommandBufferDescriptor>(m_impl);
    auto descriptor = m_impl->GetResourceCache()->GetDescriptor<GraphicsCommandBufferDescriptor>(resourceID);
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
