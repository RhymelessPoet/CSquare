#include "GraphicsPipeline.h"
#include "GraphicsPipelineDescriptor.h"
#include "GraphicsShaderStage.h"
#include "VertexInputLayout.h"

namespace CS
{
GraphicsPipeline::GraphicsPipeline(GraphicsPipelineDescriptor* descriptor) : IGraphicsResource(descriptor) {}

GraphicsPipeline& GraphicsPipeline::SetShaderStage(std::unique_ptr<GraphicsShaderStage> shaderStage)
{
    if (!IsValid()) {
        return *this;
    }
    descriptor<GraphicsPipelineDescriptor>().SetShaderStage(std::move(shaderStage));
    return *this;
}
GraphicsPipeline& GraphicsPipeline::SetVertexInputLayout(std::shared_ptr<VertexInputLayout> vertexInputLayout)
{
    if (!IsValid()) {
        return *this;
    }
    descriptor<GraphicsPipelineDescriptor>().SetVertexInputLayout(std::move(vertexInputLayout));
    return *this;
}

GraphicsPipeline& GraphicsPipeline::SetDepthTest(bool enable)
{
    if (!IsValid()) {
        return *this;
    }
    descriptor<GraphicsPipelineDescriptor>().SetDepthTest(enable);
    return *this;
}

GraphicsPipeline& GraphicsPipeline::SetDepthCompareOP(DepthCompareOp op)
{
    if (!IsValid()) {
        return *this;
    }
    descriptor<GraphicsPipelineDescriptor>().SetDepthCompareOp(op);
    return *this;
}

GraphicsPipeline& GraphicsPipeline::SetDepthClamp(bool enable)
{
    if (!IsValid()) {
        return *this;
    }
    descriptor<GraphicsPipelineDescriptor>().SetDepthClamp(enable);
    return *this;
}

GraphicsPipeline& GraphicsPipeline::SetBlendEnable(bool enable)
{
    if (!IsValid()) {
        return *this;
    }
    descriptor<GraphicsPipelineDescriptor>().SetBlendEnable(enable);
    return *this;
}

GraphicsPipeline& GraphicsPipeline::SetBlendColorFactors(BlendFactor src, BlendFactor dst)
{
    if (!IsValid()) {
        return *this;
    }
    descriptor<GraphicsPipelineDescriptor>().SetBlendColorFactors(src, dst);
    return *this;
}

GraphicsPipeline& GraphicsPipeline::SetBlendAlphaFactors(BlendFactor src, BlendFactor dst)
{
    if (!IsValid()) {
        return *this;
    }
    descriptor<GraphicsPipelineDescriptor>().SetBlendAlphaFactors(src, dst);
    return *this;
}

GraphicsPipeline& GraphicsPipeline::SetBlendColorOp(BlendOp op)
{
    if (!IsValid()) {
        return *this;
    }
    descriptor<GraphicsPipelineDescriptor>().SetBlendColorOp(op);
    return *this;
}

GraphicsPipeline& GraphicsPipeline::SetBlendAlphaOp(BlendOp op)
{
    if (!IsValid()) {
        return *this;
    }
    descriptor<GraphicsPipelineDescriptor>().SetBlendAlphaOp(op);
    return *this;
}

} // namespace CS
