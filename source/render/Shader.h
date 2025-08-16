#pragma once
#include <string>

namespace CS
{

class Shader
{
public:
    enum class Stage : uint16_t
    {
        Vertex,
        TessellationControl,
        TessellationEvaluation,
        Fragment,
        Geometry,
        Compute,
        Max
    };
    Shader(std::string source, Stage stage);
    ~Shader();

    Stage GetStage() const { return m_stage; }
    const std::string& GetSource() const { return m_source; }

private:
    std::string m_source;
    Stage m_stage;
};

using ShaderStage = Shader::Stage;

} // namespace CS
