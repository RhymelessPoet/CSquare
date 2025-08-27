#include "GraphicsPipelineDescriptor.h"
#include "GraphicsShaderStage.h"
#include "VertexInputLayout.h"
#include "graphics/GraphicsGLImpl.h"
#include "materials/Shader.h"


namespace CS
{
GraphicsPipelineDescriptor::GraphicsPipelineDescriptor(size_t id, std::shared_ptr<GraphicsGLImpl> graphicsAPI)
    : IGraphicsResourceDescriptor(id, std::move(graphicsAPI))
{}

bool GraphicsPipelineDescriptor::IsBuild() const
{
    if (m_nativePipelineData.has_value()) {
        return GetGraphicsAPI()->IsBuild(this);
    }
    return false;
}

void GraphicsPipelineDescriptor::Destroy() {}

bool GraphicsPipelineDescriptor::IsDirty() const
{
    return IGraphicsResourceDescriptor::IsDirty();
}

void GraphicsPipelineDescriptor::SetShaderStage(std::unique_ptr<GraphicsShaderStage> shaderStage)
{
    m_shaderStages.emplace(shaderStage->GetShader()->GetStage(), std::move(shaderStage));
    setDirty();
}

void GraphicsPipelineDescriptor::SetVertexInputLayout(std::unique_ptr<VertexInputLayout> layout)
{
    m_vertexInputLayout = std::move(layout);
    setDirty();
}

const std::unique_ptr<GraphicsShaderStage>& GraphicsPipelineDescriptor::GetShaderStage(ShaderStage stage)
{
    auto itr = m_shaderStages.find(stage);
    if (itr != m_shaderStages.end()) {
        return itr->second;
    }

    static std::unique_ptr<GraphicsShaderStage> defaultStage = nullptr;
    return defaultStage;
}

bool GraphicsPipelineDescriptor::build()
{
    return GetGraphicsAPI()->BuildGraphicsPipeline(this);
}

} // namespace CS
