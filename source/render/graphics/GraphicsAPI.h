#pragma once
#include "GraphicsCommandBuffer.h"
#include "RenderTarget.h"
#include "Texture.h"
#include <memory>

namespace CS
{
class GraphicsGLImpl;
class GraphicsResourceCache;

class GraphicsAPI final : public std::enable_shared_from_this<GraphicsAPI>
{
public:
    friend class IGraphicsResource;
    ~GraphicsAPI() = default;
    static std::shared_ptr<GraphicsAPI> Create(std::unique_ptr<GraphicsGLImpl> impl,
                                               std::shared_ptr<GraphicsResourceCache> resourceCache);

    void Initialize();

    Texture CreateTexture();
    Texture GetTexture(size_t id) const;
    Texture GetColorAttachment(RenderTarget renderTarget) const;

    RenderTarget CreateRenderTarget(const Size2U& size);

    GraphicsCommandBuffer CreateCommandBuffer();

    void SubmitCommandBuffer(GraphicsCommandBuffer commandBuffer);

private:
    GraphicsAPI(std::unique_ptr<GraphicsGLImpl> impl, std::shared_ptr<GraphicsResourceCache> resourceCache);

private:
    std::shared_ptr<GraphicsGLImpl> m_impl;
};

} // namespace CS
