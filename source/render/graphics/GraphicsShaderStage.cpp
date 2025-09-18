#include "GraphicsShaderStage.h"

namespace CS
{
GraphicsShaderStage::GraphicsShaderStage(std::shared_ptr<Shader> shader) : m_shader(std::move(shader)) {}

} // namespace CS
