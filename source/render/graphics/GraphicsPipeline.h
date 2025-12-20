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

private:
    GraphicsPipeline(GraphicsPipelineDescriptor* descriptor);
};

} // namespace CS
