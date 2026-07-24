#pragma once
#include "GraphicsResourceParameters.h"
#include "IGraphicsResource.h"

namespace CS
{
class GraphicsPipelineDescriptor;
class GraphicsShaderStage;
class VertexInputLayout;

class GraphicsPipeline final : public IGraphicsResource
{
public:
    friend class GraphicsAPI;
    GraphicsPipeline(/* args */) = default;
    ~GraphicsPipeline() = default;

    GraphicsPipeline& SetShaderStage(std::unique_ptr<GraphicsShaderStage> shaderStage);
    GraphicsPipeline& SetVertexInputLayout(std::shared_ptr<VertexInputLayout> vertexInputLayout);
    GraphicsPipeline& SetDepthTest(bool enable);
    GraphicsPipeline& SetDepthCompareOP(DepthCompareOp op);
    // Toggle GL_DEPTH_CLAMP-style behaviour: when enabled the rasterizer
    // clamps gl_Position.z to the depth range rather than clipping against
    // the near/far planes. Disabled by default.
    GraphicsPipeline& SetDepthClamp(bool enable);

    // Blend configuration. Blending is disabled by default; call
    // SetBlendEnable(true) plus SetBlendColorFactors/SetBlendAlphaFactors to
    // enable. Standard non-premultiplied alpha blending over an opaque
    // destination is:
    //     SetBlendEnable(true);
    //     SetBlendColorFactors(BlendFactor::SrcAlpha, BlendFactor::OneMinusSrcAlpha);
    //     SetBlendAlphaFactors(BlendFactor::One,      BlendFactor::OneMinusSrcAlpha);
    GraphicsPipeline& SetBlendEnable(bool enable);
    GraphicsPipeline& SetBlendColorFactors(BlendFactor src, BlendFactor dst);
    GraphicsPipeline& SetBlendAlphaFactors(BlendFactor src, BlendFactor dst);
    GraphicsPipeline& SetBlendColorOp(BlendOp op);
    GraphicsPipeline& SetBlendAlphaOp(BlendOp op);

    // Rasterizer face-culling configuration. Maps to glEnable(GL_CULL_FACE) +
    // glCullFace(...) on OpenGL. Defaults to CullMode::None (no culling).
    // Use CullMode::Front for shadow-caster passes to reduce peter-panning.
    GraphicsPipeline& SetCullMode(CullMode mode);

private:
    GraphicsPipeline(GraphicsPipelineDescriptor* descriptor);
};

} // namespace CS
