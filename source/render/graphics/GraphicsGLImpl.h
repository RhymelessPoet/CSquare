#pragma once
#include "GraphicsResourceDescriptors.h"
#include "base/Color.h"

namespace CS
{
class GraphicsResourceCache;
class IGraphicsResourceDescriptor;
class GraphicsPipelineDescriptor;
class GraphicsInputAssemblyDescriptor;
class OpenGLContext;
class GraphicsBufferDescriptor;
class ShaderBindingSetDescriptor;
class ShaderBinding;
class GraphicsGLImpl
{
public:
    class CurrentStates
    {
    public:
        CurrentStates() = default;
        ~CurrentStates();

        void SetPipeline(GraphicsPipelineDescriptor* pipeline);
        GraphicsPipelineDescriptor* GetPipeline() const { return m_pipeline; }

        void SetInputAssembly(GraphicsInputAssemblyDescriptor* inputAssembly);
        GraphicsInputAssemblyDescriptor* GetInputAssembly() const { return m_inputAssembly; }

        void Reset();

    private:
        GraphicsPipelineDescriptor* m_pipeline{nullptr};
        GraphicsInputAssemblyDescriptor* m_inputAssembly{nullptr};
    };
    GraphicsGLImpl(std::unique_ptr<OpenGLContext> context, std::shared_ptr<GraphicsResourceCache> resourceCache);
    ~GraphicsGLImpl();

    GraphicsGLImpl(const GraphicsGLImpl&) = delete;

    bool Initialize();

    bool SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height);

    bool BuildGraphicsBuffer(GraphicsBufferDescriptor* descriptor);
    bool UpdateGraphicsBufferData(GraphicsBufferDescriptor* descriptor, const void* data);
    bool DestroyGraphicsBuffer(GraphicsBufferDescriptor* descriptor);

    bool IsBuild(const GraphicsInputAssemblyDescriptor* descriptor);
    bool BuildGraphicsInputAssembly(GraphicsInputAssemblyDescriptor* descriptor);
    bool BindGraphicsInputAssembly(GraphicsInputAssemblyDescriptor* descriptor);
    bool DestroyGraphicsInputAssembly(GraphicsInputAssemblyDescriptor* descriptor);

    bool BindRenderTarget(RenderTargetDescriptor* descriptor);
    bool BuildRenderTarget(RenderTargetDescriptor* descriptor);
    bool DestroyRenderTarget(RenderTargetDescriptor* descriptor);

    bool IsBuild(const GraphicsPipelineDescriptor* descriptor);
    bool BuildGraphicsPipeline(GraphicsPipelineDescriptor* descriptor);
    bool BindGraphicsPipeline(GraphicsPipelineDescriptor* descriptor);

    bool BindShaderBindingSet(ShaderBindingSetDescriptor* descriptor);

    bool BuildTexture(TextureDescriptor* descriptor);
    bool UpdateTextureData(TextureDescriptor* descriptor, const void* data);
    bool DestroyTexture(TextureDescriptor* descriptor);

    bool BuildSampler(SamplerDescriptor* descriptor);
    bool DestroySampler(SamplerDescriptor* descriptor);

    bool Clear(std::optional<Color> color, std::optional<float> depth = std::nullopt);

    bool DrawIndexed(uint32_t indexCount,
                     uint32_t instanceCount = 1,
                     uint32_t firstIndex = 0,
                     int32_t vertexOffset = 0,
                     uint32_t firstInstance = 0);

    bool ResetCurrentState();

    std::shared_ptr<GraphicsResourceCache> GetResourceCache();

    template <typename DescriptorType>
    DescriptorType* GetResourceDescriptor(size_t id)
    {
        return dynamic_cast<DescriptorType*>(getIResourceDescriptor(id));
    }

private:
    IGraphicsResourceDescriptor* getIResourceDescriptor(size_t id);
    bool bindUniformBuffer(ShaderBindingSetDescriptor* descriptor, const ShaderBinding& binding);
    bool bindSampledTexture(ShaderBindingSetDescriptor* descriptor, const ShaderBinding& binding);

private:
    std::unique_ptr<OpenGLContext> m_glContext;
    std::weak_ptr<GraphicsResourceCache> m_resouceCache;
    CurrentStates m_curentStates;
};

} // namespace CS
