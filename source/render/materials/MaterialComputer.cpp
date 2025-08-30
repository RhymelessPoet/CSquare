#include "MaterialComputer.h"
#include "Material.h"
#include "graphics/GraphicsAPI.h"
#include "graphics/GraphicsShaderStage.h"
#include <cassert>

namespace CS
{

std::map<size_t, GraphicsPipeline> MaterialComputer::PipelineCache;

MaterialComputer::MaterialComputer(std::shared_ptr<GraphicsAPI> graphicsAPI) : m_graphicsAPI(std::move(graphicsAPI)) {}

MaterialComputer& MaterialComputer::Begin()
{
    assert(!m_states.has_value());

    m_states = States{};
    m_states.value().pipeline = m_graphicsAPI->CreatePipeline();

    return *this;
}

MaterialComputer& MaterialComputer::SetAttributes(const std::map<uint32_t, VertexInputFormat>& attributes)
{
    assert(m_states.has_value());
    m_states.value().pipeline.SetVertexInputLayout(m_inputLayout);

    return *this;
}

MaterialComputer& MaterialComputer::SetShaderStages(const std::vector<std::shared_ptr<Shader>>& shaders)
{
    assert(m_states.has_value());
    auto pipeline = m_states.value().pipeline;

    for (const auto& shader : shaders) {
        pipeline.SetShaderStage(std::make_unique<GraphicsShaderStage>(shader));
    }
    return *this;
}

void MaterialComputer::End(size_t id)
{
    assert(m_states.has_value());
    PipelineCache[id] = m_states.value().pipeline;

    m_states.reset();
}

GraphicsPipeline MaterialComputer::GetPipeline(const Material& material)
{
    auto pipeline = PipelineCache[material.GetID()];
    if (pipeline.IsValid()) {
        return pipeline;
    }

    material.Compute(*this);

    return PipelineCache[material.GetID()];
}

} // namespace CS
