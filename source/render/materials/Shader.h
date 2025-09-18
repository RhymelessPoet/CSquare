#pragma once
#include "graphics/ShaderBinding.h"
#include "graphics/ShaderStage.h"
#include <string>
#include <vector>

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

    const std::vector<ShaderBinding>& GetBindings() const { return m_bindings; }

private:
    std::vector<ShaderBinding> m_bindings;
    std::string m_source;
    ShaderStage m_stage;
};

} // namespace CS
