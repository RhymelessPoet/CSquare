#include "GraphicsResourceManager.h"
#include "graphics/GraphicsAPI.h"
#include "resources/MaterialGeneralUniformMemory.h"
#include "resources/MaterialTexturesMap.h"

namespace CS
{
GraphicsResourceManager::GraphicsResourceManager(std::shared_ptr<GraphicsAPI> graphicsAPI)
{
    m_materialGeneralUniforms = std::make_unique<MaterialGeneralUniformMemory>(graphicsAPI);
    m_texturesMap = std::make_unique<MaterialTexturesMap>(std::move(graphicsAPI));
}

GraphicsResourceManager::~GraphicsResourceManager() = default;

void GraphicsResourceManager::UpdateResources()
{
    if (m_texturesMap != nullptr) {
        m_texturesMap->UpdateTextures();
    }
    if (m_materialGeneralUniforms != nullptr) {
        m_materialGeneralUniforms->UpdateUniformBuffer();
    }
}

MaterialGeneralUniformMemory& GraphicsResourceManager::GetMaterialGeneralUniforms()
{
    return *m_materialGeneralUniforms;
}

MaterialTexturesMap& GraphicsResourceManager::GetMaterialTextures()
{
    return *m_texturesMap;
}

bool GraphicsResourceManager::EmplacePipeline(uint16_t materialID, GraphicsPipeline pipeline)
{
    return m_pipelines.emplace(materialID, std::move(pipeline)).second;
}

bool GraphicsResourceManager::EmplaceShaderBindingSetLayout(uint16_t materialID, ShaderBindingSetLayout layout)
{
    return m_shaderBindingSetLayouts.emplace(materialID, std::move(layout)).second;
}

bool GraphicsResourceManager::EmplaceShaderBindingSet(uint16_t materialID,
                                                      uint32_t instanceID,
                                                      ShaderBindingSet bindingSet)
{
    return m_shaderBindingSets.emplace(std::make_pair(materialID, instanceID), std::move(bindingSet)).second;
}

bool GraphicsResourceManager::RemovePipeline(uint16_t materialID)
{
    return m_pipelines.erase(materialID) > 0;
}

bool GraphicsResourceManager::RemoveShaderBindingSetLayout(uint16_t materialID)
{
    return m_shaderBindingSetLayouts.erase(materialID) > 0;
}

bool GraphicsResourceManager::RemoveShaderBindingSet(uint16_t materialID, uint32_t instanceID)
{
    return m_shaderBindingSets.erase(std::make_pair(materialID, instanceID)) > 0;
}

GraphicsPipeline GraphicsResourceManager::GetPipeline(uint16_t materialID) const
{
    auto it = m_pipelines.find(materialID);
    if (it != m_pipelines.end()) {
        return it->second;
    }
    return GraphicsPipeline();
}

ShaderBindingSetLayout GraphicsResourceManager::GetShaderBindingSetLayout(uint16_t materialID) const
{
    auto it = m_shaderBindingSetLayouts.find(materialID);
    if (it != m_shaderBindingSetLayouts.end()) {
        return it->second;
    }
    return ShaderBindingSetLayout();
}

ShaderBindingSet GraphicsResourceManager::GetShaderBindingSet(uint16_t materialID, uint32_t instanceID) const
{
    auto it = m_shaderBindingSets.find(std::make_pair(materialID, instanceID));
    if (it != m_shaderBindingSets.end()) {
        return it->second;
    }
    return ShaderBindingSet();
}

} // namespace CS
