#pragma once
#include "graphics/GraphicsPipeline.h"
#include "graphics/ShaderStage.h"
#include "graphics/VertexInputFormat.h"
#include <map>
#include <memory>
#include <optional>
#include <vector>

namespace CS
{
class GraphicsAPI;
class Shader;
class Material;

class MaterialComputer
{
public:
    struct States
    {
        GraphicsPipeline pipeline;
    };
    MaterialComputer(std::shared_ptr<GraphicsAPI> graphicsAPI);

    void SetVertexInputLayout(std::shared_ptr<VertexInputLayout> layout) { m_inputLayout = std::move(layout); }

    MaterialComputer& Begin();

    MaterialComputer& SetAttributes(const std::map<uint32_t, VertexInputFormat>& attributes);
    MaterialComputer& SetShaderStages(const std::vector<std::shared_ptr<Shader>>& shaders);

    void End(size_t id);

    GraphicsPipeline GetPipeline(const Material& material);

private:
    std::optional<States> m_states;
    std::shared_ptr<VertexInputLayout> m_inputLayout;
    std::shared_ptr<GraphicsAPI> m_graphicsAPI;

    static std::map<size_t, GraphicsPipeline> PipelineCache;
};

} // namespace CS
