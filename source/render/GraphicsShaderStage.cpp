#include "GraphicsShaderStage.h"

namespace CS
{
GraphicsShaderStage::GraphicsShaderStage(std::shared_ptr<Shader> shader) : m_shader(std::move(shader)) {}

GraphicsShaderStage& GraphicsShaderStage::SetResourceBinding(uint32_t binding, Texture texture)
{
    return *this;
}

GraphicsShaderStage& GraphicsShaderStage::SetResourceBinding(uint32_t binding, UniformBuffer uniformBuffer)
{
    return *this;
}

} // namespace CS
