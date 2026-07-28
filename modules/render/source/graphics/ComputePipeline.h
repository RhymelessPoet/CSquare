#pragma once
#include "IGraphicsResource.h"
#include <memory>

namespace CS
{
class ComputePipelineDescriptor;
class GraphicsShaderStage;

class ComputePipeline final : public IGraphicsResource
{
public:
    friend class GraphicsAPI;
    ComputePipeline() = default;
    ~ComputePipeline() = default;

    ComputePipeline& SetShaderStage(std::unique_ptr<GraphicsShaderStage> shaderStage);

private:
    explicit ComputePipeline(ComputePipelineDescriptor* descriptor);
};
} // namespace CS
