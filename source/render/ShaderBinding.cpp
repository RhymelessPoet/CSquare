#include "ShaderBinding.h"

namespace CS
{

ShaderBinding::ShaderBinding(uint32_t binding, ShaderStageFlags stages, Type type)
    : m_binding(binding), m_stages(stages), m_type(type)
{}

ShaderBinding::ShaderBinding(uint32_t binding, ShaderStage stage, Type type)
    : m_binding(binding), m_stages(static_cast<ShaderStageFlags>(stage)), m_type(type)
{}

} // namespace CS
