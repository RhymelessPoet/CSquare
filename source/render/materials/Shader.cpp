#include "Shader.h"

namespace CS
{

Shader::Shader(std::string source, ShaderStage stage) : m_source(std::move(source)), m_stage(stage) {}

Shader::~Shader() {}

void Shader::AddBinding(const ShaderBinding& binding)
{
    m_bindings.emplace(binding.GetBinding(), binding);
}

} // namespace CS
