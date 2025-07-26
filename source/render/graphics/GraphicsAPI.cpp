#include "GraphicsAPI.h"
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

RenderTarget GraphicsAPI::CreateRenderTarget(const Size2U& size)
{
    auto resourceID = m_impl->GetResourceCache()->Allocate<RenderTargetDescriptor>(m_impl);
    auto descriptor = m_impl->GetResourceCache()->GetDescriptor<RenderTargetDescriptor>(resourceID);
    return RenderTarget(descriptor);
}

} // namespace CS
