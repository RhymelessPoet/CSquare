#pragma once
#include "IGraphicsResourceDescriptor.h"
#include "Shader.h"
#include <any>
#include <map>

namespace CS
{
class GraphicsShaderStage;
class VertexInputLayout;

class GraphicsPipelineDescriptor : public IGraphicsResourceDescriptor
{
public:
    GraphicsPipelineDescriptor(size_t id, std::shared_ptr<GraphicsGLImpl> graphicsAPI);
    ~GraphicsPipelineDescriptor() = default;

    virtual bool IsBuild() const override;
    virtual void Destroy() override;
    virtual bool IsDirty() const override;

    void SetShaderStage(std::unique_ptr<GraphicsShaderStage> shaderStage);
    const std::unique_ptr<GraphicsShaderStage>& GetShaderStage(ShaderStage stage);
    const VertexInputLayout& GetVertexInputLayout() const { return *m_vertexInputLayout.get(); }
    void SetVertexInputLayout(std::unique_ptr<VertexInputLayout> layout);

    template <typename PipelineNativeDataType>
    PipelineNativeDataType GetNativePipelineData() const
    {
        return std::any_cast<PipelineNativeDataType>(m_nativePipelineData);
    }

    template <typename PipelineNativeDataType>
    void SetNativePipelineData(PipelineNativeDataType&& data)
    {
        m_nativePipelineData = std::forward<PipelineNativeDataType>(data);
    }

protected:
    virtual bool build() override;

private:
    std::unique_ptr<VertexInputLayout> m_vertexInputLayout;
    std::map<ShaderStage, std::unique_ptr<GraphicsShaderStage>> m_shaderStages;
    std::any m_nativePipelineData;
};

} // namespace CS
