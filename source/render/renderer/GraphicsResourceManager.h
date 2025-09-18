#pragma once
#include "graphics/GraphicsPipeline.h"
#include "graphics/ShaderBindingSet.h"
#include "graphics/ShaderBindingSetLayout.h"
#include <map>
#include <memory>

namespace CS
{
class MaterialGeneralUniformMemory;
class MaterialTexturesMap;
class GraphicsAPI;

class GraphicsResourceManager
{
public:
    GraphicsResourceManager(std::shared_ptr<GraphicsAPI> graphicsAPI);
    ~GraphicsResourceManager();

    void UpdateResources();

    MaterialGeneralUniformMemory& GetMaterialGeneralUniforms();
    MaterialTexturesMap& GetMaterialTextures();

    bool EmplacePipeline(uint16_t materialID, GraphicsPipeline pipeline);
    bool EmplaceShaderBindingSetLayout(uint16_t materialID, ShaderBindingSetLayout layout);
    bool EmplaceShaderBindingSet(uint16_t materialID, uint32_t instanceID, ShaderBindingSet bindingSet);

    bool RemovePipeline(uint16_t materialID);
    bool RemoveShaderBindingSetLayout(uint16_t materialID);
    bool RemoveShaderBindingSet(uint16_t materialID, uint32_t instanceID);

    GraphicsPipeline GetPipeline(uint16_t materialID) const;
    ShaderBindingSetLayout GetShaderBindingSetLayout(uint16_t materialID) const;
    ShaderBindingSet GetShaderBindingSet(uint16_t materialID, uint32_t instanceID) const;

private:
    std::unique_ptr<MaterialGeneralUniformMemory> m_materialGeneralUniforms;
    std::unique_ptr<MaterialTexturesMap> m_texturesMap;

    std::map<uint16_t, GraphicsPipeline> m_pipelines;
    std::map<uint16_t, ShaderBindingSetLayout> m_shaderBindingSetLayouts;
    std::map<std::pair<uint16_t, uint32_t>, ShaderBindingSet> m_shaderBindingSets;
};

} // namespace CS
