#include "ComputePipelineDescriptor.h"
#include "GraphicsShaderStage.h"
#include "GraphicsGLImpl.h"
#include "materials/Shader.h"

namespace CS
{
ComputePipelineDescriptor::ComputePipelineDescriptor(size_t id, std::shared_ptr<GraphicsGLImpl> graphicsAPI)
    : IGraphicsResourceDescriptor(id, std::move(graphicsAPI))
{}

bool ComputePipelineDescriptor::IsBuild() const
{
    auto graphicsAPI = GetGraphicsAPI();
    return graphicsAPI != nullptr && m_nativePipelineData.has_value() && graphicsAPI->IsBuild(this);
}

void ComputePipelineDescriptor::Destroy()
{
    auto graphicsAPI = GetGraphicsAPI();
    if (graphicsAPI != nullptr && m_nativePipelineData.has_value()) {
        graphicsAPI->DestroyComputePipeline(this);
    }
}

void ComputePipelineDescriptor::SetShaderStage(std::unique_ptr<GraphicsShaderStage> shaderStage)
{
    if (shaderStage && shaderStage->GetShader()->GetStage() == ShaderStage::Compute) {
        m_shaderStage = std::move(shaderStage);
        setDirty();
    }
}

bool ComputePipelineDescriptor::build()
{
    return GetGraphicsAPI()->BuildComputePipeline(this);
}
} // namespace CS
