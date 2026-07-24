#pragma once
#include "Texture.h"
#include "UniformBuffer.h"
#include <map>
#include <memory>

namespace CS
{
class Shader;

class GraphicsShaderStage
{
public:
    GraphicsShaderStage(std::shared_ptr<Shader> shader);
    ~GraphicsShaderStage() = default;

    const Shader& GetShader() const { return *m_shader; }
    std::shared_ptr<Shader> GetShader() { return m_shader; }

private:
    std::shared_ptr<Shader> m_shader;
    std::map<uint32_t, Texture> m_resourceBindings;
};

} // namespace CS
