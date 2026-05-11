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
        // When true, rasterizer clamps gl_Position.z into [near, far] instead
        // of clipping primitives against the near/far planes. Equivalent to
        // GL_DEPTH_CLAMP in OpenGL. Useful for "infinite" geometry (skyboxes,
        // infinite grids) whose far horizon would otherwise be near-plane
        // clipped at the frustum boundary.
        bool depthClampEnable : 1 {false};
    };

    struct BlendState
    {
        BlendFactor srcColorFactor{BlendFactor::One};
        BlendFactor dstColorFactor{BlendFactor::Zero};
        BlendOp colorOp{BlendOp::Add};
        BlendFactor srcAlphaFactor{BlendFactor::One};
        BlendFactor dstAlphaFactor{BlendFactor::Zero};
        BlendOp alphaOp{BlendOp::Add};
        bool blendEnable : 1 {false};
    };

    // Rasterizer-state fragment related to face culling. Defaults to no culling
    // so that existing materials keep their pre-encapsulation behaviour.
    struct RasterizationState
    {
        CullMode cullMode{CullMode::None};
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

    void SetDepthClamp(bool enable) { m_depthStencilState.depthClampEnable = enable; }
    bool IsDepthClampEnabled() const { return m_depthStencilState.depthClampEnable; }

    const DepthStencilState& GetDepthStencilState() const { return m_depthStencilState; }

    void SetBlendEnable(bool enable) { m_blendState.blendEnable = enable; }
    void SetBlendColorFactors(BlendFactor src, BlendFactor dst)
    {
        m_blendState.srcColorFactor = src;
        m_blendState.dstColorFactor = dst;
    }
    void SetBlendAlphaFactors(BlendFactor src, BlendFactor dst)
    {
        m_blendState.srcAlphaFactor = src;
        m_blendState.dstAlphaFactor = dst;
    }
    void SetBlendColorOp(BlendOp op) { m_blendState.colorOp = op; }
    void SetBlendAlphaOp(BlendOp op) { m_blendState.alphaOp = op; }
    const BlendState& GetBlendState() const { return m_blendState; }

    void SetCullMode(CullMode mode) { m_rasterizationState.cullMode = mode; }
    CullMode GetCullMode() const { return m_rasterizationState.cullMode; }
    const RasterizationState& GetRasterizationState() const { return m_rasterizationState; }

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
    BlendState m_blendState;
    RasterizationState m_rasterizationState;
    std::any m_nativePipelineData;
};

} // namespace CS
