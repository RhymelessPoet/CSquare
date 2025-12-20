#include "MaterialCompiler.h"
#include "GraphicsResourceManager.h"
#include "MaterialUniformIDCreator.h"
#include "graphics/GraphicsAPI.h"
#include "graphics/GraphicsShaderStage.h"
#include "materials/IMaterialConfiguration.h"
#include "materials/ImageTexture.h"
#include "materials/Material.h"
#include "materials/Shader.h"
#include "resources/MaterialGeneralUniformMemory.h"
#include "resources/MaterialTexturesMap.h"
#include <cassert>

namespace CS
{

MaterialCompiler::MaterialCompiler(std::shared_ptr<GraphicsAPI> graphicsAPI,
                                   std::shared_ptr<GraphicsResourceManager> resourceManager)
    : m_graphicsAPI(graphicsAPI), m_resourceManager(std::move(resourceManager))
{
    m_uniformIDCreator = std::make_unique<MaterialUniformIDCreator>();
}

MaterialCompiler::~MaterialCompiler() = default;

MaterialCompiler& MaterialCompiler::BeginMaterial()
{
    assert(!m_materialStates.has_value());

    m_materialStates = MaterialStates{};
    m_materialStates.value().pipeline = m_graphicsAPI->CreatePipeline();
    m_materialStates.value().bindLayout = m_graphicsAPI->CreateShaderBindingSetLayout();

    return *this;
}

MaterialCompiler& MaterialCompiler::SetConfiguration(const IMaterialConfiguration& configuration)
{
    assert(m_materialStates.has_value());
    configuration.Configure(m_materialStates.value().pipeline);

    return *this;
}

MaterialCompiler& MaterialCompiler::SetAttributes(const std::map<uint32_t, VertexInputFormat>& attributes)
{
    assert(m_materialStates.has_value());
    m_materialStates.value().pipeline.SetVertexInputLayout(m_inputLayout);

    return *this;
}

MaterialCompiler& MaterialCompiler::SetShaderStages(const std::vector<std::shared_ptr<Shader>>& shaders)
{
    assert(m_materialStates.has_value());
    auto pipeline = m_materialStates.value().pipeline;
    auto bindLayout = m_materialStates.value().bindLayout;
    auto& bindings = m_materialStates.value().bindings;

    for (const auto& shader : shaders) {
        pipeline.SetShaderStage(std::make_unique<GraphicsShaderStage>(shader));
        for (const auto& binding : shader->GetBindings()) {
            collectShaderbinding(bindings, binding);
        }
    }

    for (const auto& [_, binding] : bindings) {
        bindLayout.AddBinding(binding);
    }

    return *this;
}

void MaterialCompiler::EndMaterial(uint16_t id)
{
    assert(m_materialStates.has_value());
    m_resourceManager->EmplacePipeline(id, m_materialStates.value().pipeline);
    m_resourceManager->EmplaceShaderBindingSetLayout(id, m_materialStates.value().bindLayout);

    m_materialStates.reset();
}

GraphicsPipeline MaterialCompiler::GetPipeline(const Material& material)
{
    auto pipeline = m_resourceManager->GetPipeline(material.GetID());
    if (pipeline.IsValid()) {
        return pipeline;
    }

    material.Compile(*this);

    return m_resourceManager->GetPipeline(material.GetID());
}

ShaderBindingSetLayout MaterialCompiler::GetBindingSetLayout(const Material& material)
{
    auto layout = m_resourceManager->GetShaderBindingSetLayout(material.GetID());
    if (layout.IsValid()) {
        return layout;
    }

    material.Compile(*this);

    return m_resourceManager->GetShaderBindingSetLayout(material.GetID());
}

MaterialCompiler&
MaterialCompiler::BeginMaterialInstance(uint16_t materialID, uint32_t instanceID, ShaderBindingSetLayout layout)
{
    assert(!m_materialInstanceStates.has_value());

    m_materialInstanceStates = MaterialInstanceStates{.materialID = materialID,
                                                      .instanceID = instanceID,
                                                      .bindingSet = m_graphicsAPI->CreateShaderBindingSet(layout)};

    return *this;
}

MaterialCompiler& MaterialCompiler::SetUniformBuffer(uint32_t binding, const MaterialInstance::Uniforms& uniforms)
{
    assert(m_materialInstanceStates.has_value());

    auto [materialID, instanceID, bindingSet] = m_materialInstanceStates.value();
    auto size = bindingSet.GetBindingSize(binding);

    auto& generalUniformMemory = m_resourceManager->GetMaterialGeneralUniforms();
    auto baseOffset = generalUniformMemory.Allocate(size);
    for (const auto& [name, uniform] : uniforms) {
        auto uniformSize = ByteSizeOf(uniform.value);
        auto uniformID = m_uniformIDCreator->GetUniformIdentifier(materialID, instanceID, name);
        auto uniformData = generalUniformMemory.AllocateUniform(uniformID, baseOffset + uniform.offset, uniformSize);
        auto bytes = ToBytes(uniform.value);
        assert(bytes.size() == uniformData.size());
        std::copy(bytes.begin(), bytes.end(), uniformData.begin());
    }

    bindingSet.BindUniformBuffer(binding, generalUniformMemory.GetUniformBuffer(), baseOffset, size);

    return *this;
}

MaterialCompiler& MaterialCompiler::SetTexture(const MaterialInstance::Textures::value_type& uniform)
{
    assert(m_materialInstanceStates.has_value());

    auto [materialID, instanceID, bindingSet] = m_materialInstanceStates.value();
    auto uniformID = m_uniformIDCreator->GetUniformIdentifier(materialID, instanceID, uniform.first);

    auto& textureMap = m_resourceManager->GetMaterialTextures();

    auto& [materialTexture, binding, _] = uniform.second;
    auto imageTexture = dynamic_cast<const ImageTexture*>(materialTexture.get());
    if (imageTexture != nullptr) {
        auto sampledTexture = textureMap.AllocateTexture(uniformID, *imageTexture);
        bindingSet.BindSampledTexture(binding, sampledTexture->texture, sampledTexture->sampler);
    } else {
        // TODO: log error
    }

    return *this;
}

void MaterialCompiler::EndMaterialInstance()
{
    assert(m_materialInstanceStates.has_value());

    auto [materialID, instanceID, bindingSet] = m_materialInstanceStates.value();
    m_resourceManager->EmplaceShaderBindingSet(materialID, instanceID, bindingSet);

    m_materialInstanceStates.reset();
}

ShaderBindingSet MaterialCompiler::GetShaderBindingSet(const MaterialInstance& material)
{
    if (!material.HasInstancedUniform()) {
        return GetShaderBindingSet(material.GetDefaultInstance());
    }

    auto key = std::make_pair(material.GetMaterialID(), material.GetID());
    auto bindingSet = m_resourceManager->GetShaderBindingSet(key.first, key.second);
    if (bindingSet.IsValid()) {
        return bindingSet;
    }

    material.Compile(*this);

    return m_resourceManager->GetShaderBindingSet(key.first, key.second);
}

void MaterialCompiler::Apply(uint16_t materialID, uint32_t instanceID, const MaterialInstance::Uniforms& uniforms)
{
    auto& generalUniformMemory = m_resourceManager->GetMaterialGeneralUniforms();

    for (auto& [name, uniform] : uniforms) {
        if (!uniform.dirty) {
            continue;
        }
        auto uniformSize = ByteSizeOf(uniform.value);
        auto uniformID = m_uniformIDCreator->GetUniformIdentifier(materialID, instanceID, name);
        generalUniformMemory.SetUniformMemory(uniformID, ToBytes(uniform.value));
    }
}

void MaterialCompiler::Apply(uint16_t materialID, uint32_t instanceID, const MaterialInstance::Textures& textures)
{
    auto& texturesMap = m_resourceManager->GetMaterialTextures();

    for (auto& [name, texture] : textures) {
        if (!texture.dirty) {
            continue;
        }
        auto imageTexture = dynamic_cast<ImageTexture*>(texture.texture.get());
        if (imageTexture == nullptr) {
            continue;
        }
        texturesMap.SetTextureData(name, imageTexture->GetImage());
    }
}

void MaterialCompiler::collectShaderbinding(std::map<uint32_t, ShaderBinding>& bindings, ShaderBinding binding)
{
    auto itr = bindings.find(binding.GetBinding());
    if (itr != bindings.end()) {
        if (itr->second == binding) {
            itr->second.EnableShaderStages(binding.GetShaderStages());
        } else {
            // TODO: log error
        }
    } else {
        bindings.emplace(binding.GetBinding(), std::move(binding));
    }
}

} // namespace CS
