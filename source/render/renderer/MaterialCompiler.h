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
#include <ranges>
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
class ImageTexture;
class RenderTexture;

using ViewInputs = std::views::all_t<std::map<uint32_t, Texture>&>;

class MaterialCompiler
{
public:
    struct MaterialStates
    {
        GraphicsPipeline pipeline;
        ShaderBindingSetLayout bindLayout;
        IMaterialConfiguration* configuration{nullptr};
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

    void SetInputs(ViewInputs inputs) { m_currentViewInputs = inputs; }

    // material
    MaterialCompiler& BeginMaterial(IMaterialConfiguration* configuration);
    MaterialCompiler& SetAttributes(const std::map<uint32_t, VertexInputFormat>& attributes);
    MaterialCompiler& SetShaderStages(const std::vector<std::shared_ptr<Shader>>& shaders);
    void EndMaterial(uint16_t id);

    GraphicsPipeline GetPipeline(const Material& material);
    ShaderBindingSetLayout GetBindingSetLayout(const Material& material);

    // material instance
    MaterialCompiler& BeginMaterialInstance(uint16_t materialID, uint32_t instanceID, ShaderBindingSetLayout layout);
    MaterialCompiler& SetUniformBuffer(uint32_t binding, const MaterialInstance::Uniforms& uniforms);

    MaterialCompiler& SetTexture(const IMaterialConfiguration& configuration,
                                 const MaterialInstance::Textures::value_type& uniform);
    void EndMaterialInstance();

    ShaderBindingSet GetShaderBindingSet(const MaterialInstance& material);

    void Apply(uint16_t materialID, uint32_t instanceID, const MaterialInstance::Uniforms& uniforms);
    void Apply(const IMaterialConfiguration& configuration,
               uint16_t materialID,
               uint32_t instanceID,
               const MaterialInstance::Textures& textures);

private:
    void collectShaderbinding(std::map<uint32_t, ShaderBinding>& bindings, ShaderBinding binding);
    void bindTexture(uint32_t binding, std::string_view uniformName, const ImageTexture* texture);
    void bindTexture(const IMaterialConfiguration& configuration,
                     uint32_t binding,
                     std::string_view uniformName,
                     const RenderTexture* texture);
    Texture getInput(uint32_t slot) const;

private:
    std::shared_ptr<GraphicsAPI> m_graphicsAPI;
    std::shared_ptr<GraphicsResourceManager> m_resourceManager;
    std::unique_ptr<MaterialUniformIDCreator> m_uniformIDCreator;

    ViewInputs m_currentViewInputs;

    std::shared_ptr<VertexInputLayout> m_inputLayout;

    std::optional<MaterialStates> m_materialStates;
    std::optional<MaterialInstanceStates> m_materialInstanceStates;
};

} // namespace CS
