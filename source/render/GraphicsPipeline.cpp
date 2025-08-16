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

GraphicsPipeline& GraphicsPipeline::SetVertexInputLayout(std::unique_ptr<VertexInputLayout> vertexInputLayout)
{
    if (!IsValid()) {
        return *this;
    }

    descriptor<GraphicsPipelineDescriptor>().SetVertexInputLayout(std::move(vertexInputLayout));

    return *this;
}

} // namespace CS
