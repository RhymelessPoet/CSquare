#include "Shader.h"

namespace CS
{

Shader::Shader(std::string source, Stage stage) : m_source(std::move(source)), m_stage(stage) {}

Shader::~Shader() {}

} // namespace CS
