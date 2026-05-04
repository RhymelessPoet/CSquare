#pragma once
#include "GraphicsCommandBuffer.h"
#include "GraphicsInputAssembly.h"
#include "GraphicsPipeline.h"
#include "IndexBuffer.h"
#include "RenderTarget.h"
#include "Sampler.h"
#include "ShaderBindingSet.h"
#include "ShaderBindingSetLayout.h"
#include "Texture.h"
#include "UniformBuffer.h"
#include "VertexBuffer.h"
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

    VertexBuffer CreateVertexBuffer(size_t size);
    IndexBuffer CreateIndexBuffer(size_t size);
    UniformBuffer CreateUniformBuffer(size_t size);

    GraphicsInputAssembly CreateInputAssembly();
    GraphicsPipeline CreatePipeline();

    Texture CreateTexture(TextureFormat format = TextureFormat::RGBA8Unorm, const Size2u& size = {1u, 1u});
    Texture GetTexture(size_t id) const;
    Texture GetColorAttachment(RenderTarget renderTarget) const;
    Texture GetDepthAttachment(RenderTarget renderTarget) const;
    Texture GetDepthStencilAttachment(RenderTarget renderTarget) const;

    Sampler CreateSampler();

    RenderTarget CreateRenderTarget(const Size2u& size);

    ShaderBindingSetLayout CreateShaderBindingSetLayout();
    ShaderBindingSet CreateShaderBindingSet(ShaderBindingSetLayout layout);

    GraphicsCommandBuffer CreateCommandBuffer();
    void SubmitCommandBuffer(GraphicsCommandBuffer commandBuffer);

private:
    GraphicsAPI(std::unique_ptr<GraphicsGLImpl> impl, std::shared_ptr<GraphicsResourceCache> resourceCache);

private:
    std::shared_ptr<GraphicsGLImpl> m_impl;
};

} // namespace CS
