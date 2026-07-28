#include "ComputePipeline.h"
#include "ComputePipelineDescriptor.h"
#include "GraphicsShaderStage.h"

namespace CS
{
ComputePipeline::ComputePipeline(ComputePipelineDescriptor* descriptor) : IGraphicsResource(descriptor) {}

ComputePipeline& ComputePipeline::SetShaderStage(std::unique_ptr<GraphicsShaderStage> shaderStage)
{
    if (IsValid()) {
        descriptor<ComputePipelineDescriptor>().SetShaderStage(std::move(shaderStage));
    }
    return *this;
}
} // namespace CS
