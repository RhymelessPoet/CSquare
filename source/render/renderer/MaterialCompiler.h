#pragma once
#include "graphics/GraphicsPipeline.h"
#include "graphics/ShaderBindingSet.h"
#include "graphics/ShaderBindingSetLayout.h"
#include "graphics/ShaderStage.h"
#include "graphics/VertexInputFormat.h"
#include "materials/MaterialInstance.h"
#include <map>
#include <memory>
#include <optional>
#include <vector>

namespace CS
{
class GraphicsAPI;
class Shader;
class Material;
class MaterialInstance;
class IMaterialConfiguration;
class MaterialUniformIDCreator;
class GraphicsResourceManager;

class MaterialCompiler
{
public:
    struct MaterialStates
    {
        GraphicsPipeline pipeline;
        ShaderBindingSetLayout bindLayout;
        std::map<uint32_t, ShaderBinding> bindings;
    };
    struct MaterialInstanceStates
    {
        uint16_t materialID{0u};
        uint32_t instanceID{0u};
        ShaderBindingSet bindingSet;
    };

    explicit MaterialCompiler(std::shared_ptr<GraphicsAPI> graphicsAPI,
                              std::shared_ptr<GraphicsResourceManager> resourceManager);
    ~MaterialCompiler();

    void SetVertexInputLayout(std::shared_ptr<VertexInputLayout> layout) { m_inputLayout = std::move(layout); }

    // material
    MaterialCompiler& BeginMaterial();
    MaterialCompiler& SetConfiguration(const IMaterialConfiguration& configuration);
    MaterialCompiler& SetAttributes(const std::map<uint32_t, VertexInputFormat>& attributes);
    MaterialCompiler& SetShaderStages(const std::vector<std::shared_ptr<Shader>>& shaders);
    void EndMaterial(uint16_t id);

    GraphicsPipeline GetPipeline(const Material& material);
    ShaderBindingSetLayout GetBindingSetLayout(const Material& material);

    // material instance
    MaterialCompiler& BeginMaterialInstance(uint16_t materialID, uint32_t instanceID, ShaderBindingSetLayout layout);
    MaterialCompiler& SetUniformBuffer(uint32_t binding, const MaterialInstance::Uniforms& uniforms);
    MaterialCompiler& SetTexture(const MaterialInstance::Textures::value_type& uniform);
    void EndMaterialInstance();

    ShaderBindingSet GetShaderBindingSet(const MaterialInstance& material);

    void Apply(uint16_t materialID, uint32_t instanceID, const MaterialInstance::Uniforms& uniforms);
    void Apply(uint16_t materialID, uint32_t instanceID, const MaterialInstance::Textures& textures);

private:
    void collectShaderbinding(std::map<uint32_t, ShaderBinding>& bindings, ShaderBinding binding);

private:
    std::shared_ptr<GraphicsAPI> m_graphicsAPI;
    std::shared_ptr<GraphicsResourceManager> m_resourceManager;
    std::unique_ptr<MaterialUniformIDCreator> m_uniformIDCreator;

    std::shared_ptr<VertexInputLayout> m_inputLayout;

    std::optional<MaterialStates> m_materialStates;
    std::optional<MaterialInstanceStates> m_materialInstanceStates;
};

} // namespace CS
