#pragma once
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
    GraphicsPipeline& SetVertexInputLayout(std::unique_ptr<VertexInputLayout> vertexInputLayout);

private:
    GraphicsPipeline(GraphicsPipelineDescriptor* descriptor);
};

} // namespace CS
