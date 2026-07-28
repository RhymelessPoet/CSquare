#pragma once
#include "IGraphicsResourceDescriptor.h"
#include <any>
#include <memory>

namespace CS
{
class GraphicsShaderStage;

class ComputePipelineDescriptor final : public IGraphicsResourceDescriptor
{
public:
    ComputePipelineDescriptor(size_t id, std::shared_ptr<GraphicsGLImpl> graphicsAPI);
    ~ComputePipelineDescriptor() = default;

    bool IsBuild() const override;
    void Destroy() override;
    void SetShaderStage(std::unique_ptr<GraphicsShaderStage> shaderStage);
    const std::unique_ptr<GraphicsShaderStage>& GetShaderStage() const { return m_shaderStage; }

    template <typename T> T GetNativePipelineData() const { return std::any_cast<T>(m_nativePipelineData); }
    template <typename T> void SetNativePipelineData(T&& data) { m_nativePipelineData = std::forward<T>(data); }
    void ResetNativePipelineData() { m_nativePipelineData.reset(); }

protected:
    bool build() override;

private:
    std::unique_ptr<GraphicsShaderStage> m_shaderStage;
    std::any m_nativePipelineData;
};
} // namespace CS
