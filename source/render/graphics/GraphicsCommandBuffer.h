#pragma once
#include "GraphicsInputAssembly.h"
#include "GraphicsPipeline.h"
#include "RenderTarget.h"
#include "ShaderBindingSet.h"
#include "base/Color.h"
#include <optional>

namespace CS
{

class GraphicsCommandBufferDescriptor;

class GraphicsCommandBuffer final : public IGraphicsResource
{
public:
    friend class GraphicsAPI;
    class GraphicsCommandBufferImpl;
    ~GraphicsCommandBuffer();

    GraphicsCommandBuffer& BeginPass(RenderTarget renderTarget);
    GraphicsCommandBuffer& EndPass();

    GraphicsCommandBuffer& Clear(std::optional<Color> clearColor);
    GraphicsCommandBuffer& SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height);

    GraphicsCommandBuffer& Bind(GraphicsPipeline pipeline);
    GraphicsCommandBuffer& Bind(GraphicsInputAssembly inputAssembly);
    GraphicsCommandBuffer& Bind(ShaderBindingSet shaderBindingSet);
    GraphicsCommandBuffer& DrawIndexed(uint32_t count, uint32_t indexOffset);

    void ClearBuffer();

private:
    GraphicsCommandBuffer(GraphicsCommandBufferDescriptor* descriptor);
};

} // namespace CS
