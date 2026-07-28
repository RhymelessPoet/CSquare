#pragma once
#include "GraphicsCommandBuffer.h"
#include "ComputePipeline.h"
#include "GraphicsInputAssembly.h"
#include "GraphicsPipeline.h"
#include "IndexBuffer.h"
#include "RenderTarget.h"
#include "Sampler.h"
#include "StorageBuffer.h"
#include "ShaderBindingSet.h"
#include "ShaderBindingSetLayout.h"
#include "Texture.h"
#include "UniformBuffer.h"
#include "VertexBuffer.h"
#include <memory>
#include <vector>

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

    // Context lifecycle: wrap any GL work on the current thread between
    // MakeContextCurrent()/DoneContextCurrent() when coexisting with another
    // GL context (e.g. Qt RHI).
    bool MakeContextCurrent();
    void DoneContextCurrent();

    // Present the given render target's color attachment to the default
    // framebuffer of the currently bound GL context and swap buffers.
    // Standalone samples (no Qt) call this once per frame; the editor path
    // does not use Present because Qt consumes the texture directly.
    void Present(RenderTarget source, const Size2u& windowSize);
    void SwapBuffers();

    VertexBuffer CreateVertexBuffer(size_t size);
    IndexBuffer CreateIndexBuffer(size_t size);
    UniformBuffer CreateUniformBuffer(size_t size);
    StorageBuffer CreateStorageBuffer(size_t size);
    VertexBuffer CreateVertexBufferView(StorageBuffer buffer);
    std::vector<std::byte> ReadStorageBuffer(StorageBuffer buffer, size_t offset, size_t size);

    GraphicsInputAssembly CreateInputAssembly();
    GraphicsPipeline CreatePipeline();
    ComputePipeline CreateComputePipeline();

    Texture CreateTexture(TextureFormat format = TextureFormat::RGBA8Unorm, const Size2u& size = {1u, 1u});
    Texture GetTexture(size_t id) const;
    std::vector<std::byte> ReadTexture(Texture texture);
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
