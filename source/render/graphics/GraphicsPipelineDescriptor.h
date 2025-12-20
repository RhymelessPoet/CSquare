#pragma once
#include "GraphicsResourceParameters.h"
#include "IGraphicsResourceDescriptor.h"
#include "materials/Shader.h"
#include <any>
#include <map>

namespace CS
{
class GraphicsShaderStage;
class VertexInputLayout;

class GraphicsPipelineDescriptor : public IGraphicsResourceDescriptor
{
public:
    struct DepthStencilState
    {
        DepthCompareOp depthCompareOp{DepthCompareOp::Less};
        bool depthTestEnable : 1 {true};
    };
    GraphicsPipelineDescriptor(size_t id, std::shared_ptr<GraphicsGLImpl> graphicsAPI);
    ~GraphicsPipelineDescriptor() = default;

    virtual bool IsBuild() const override;
    virtual void Destroy() override;
    virtual bool IsDirty() const override;

    void SetShaderStage(std::unique_ptr<GraphicsShaderStage> shaderStage);
    const std::unique_ptr<GraphicsShaderStage>& GetShaderStage(ShaderStage stage);
    const VertexInputLayout& GetVertexInputLayout() const { return *m_vertexInputLayout.get(); }
    void SetVertexInputLayout(std::shared_ptr<VertexInputLayout> layout);

    void SetDepthTest(bool enable) { m_depthStencilState.depthTestEnable = enable; }
    bool IsDepthTestEnabled() const { return m_depthStencilState.depthTestEnable; }

    void SetDepthCompareOp(DepthCompareOp op) { m_depthStencilState.depthCompareOp = op; }
    DepthCompareOp GetDepthCompareOp() const { return m_depthStencilState.depthCompareOp; }

    const DepthStencilState& GetDepthStencilState() const { return m_depthStencilState; }

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
    std::shared_ptr<VertexInputLayout> m_vertexInputLayout;
    std::map<ShaderStage, std::unique_ptr<GraphicsShaderStage>> m_shaderStages;
    DepthStencilState m_depthStencilState;
    std::any m_nativePipelineData;
};

} // namespace CS
