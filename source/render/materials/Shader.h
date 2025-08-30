#pragma once
#include "graphics/ShaderBinding.h"
#include "graphics/ShaderStage.h"
#include <map>
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

    void AddBinding(const ShaderBinding& binding);

private:
    std::map<uint32_t, ShaderBinding> m_bindings;
    std::string m_source;
    ShaderStage m_stage;
};

} // namespace CS
