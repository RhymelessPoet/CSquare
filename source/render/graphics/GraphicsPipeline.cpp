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

} // namespace CS
