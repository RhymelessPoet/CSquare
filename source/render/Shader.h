#pragma once
#include "ShaderStage.h"
#include <string>

namespace CS
{

class Shader
{
public:
    Shader(std::string source, ShaderStage stage);
    ~Shader();

    ShaderStage GetStage() const { return m_stage; }
    const std::string& GetSource() const { return m_source; }

private:
    std::string m_source;
    ShaderStage m_stage;
};

} // namespace CS
