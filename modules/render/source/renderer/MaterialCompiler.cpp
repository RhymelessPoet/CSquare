#include "MaterialCompiler.h"
#include "GraphicsResourceManager.h"
#include "MaterialUniformIDCreator.h"
#include "Logger.h"
#include "graphics/GraphicsAPI.h"
#include "graphics/GraphicsShaderStage.h"
#include "graphics/RenderTexture.h"
#include "materials/IMaterialConfiguration.h"
#include "materials/ImageTexture.h"
#include "materials/Material.h"
#include "materials/Shader.h"
#include "resources/MaterialGeneralUniformMemory.h"
#include "resources/MaterialTexturesMap.h"
#include <cassert>

namespace CS
{
static std::map<uint32_t, Texture> textures;

MaterialCompiler::MaterialCompiler(std::shared_ptr<GraphicsAPI> graphicsAPI,
                                   std::shared_ptr<GraphicsResourceManager> resourceManager)
    : m_graphicsAPI(graphicsAPI), m_resourceManager(std::move(resourceManager)), m_currentViewInputs(textures)
{
    m_uniformIDCreator = std::make_unique<MaterialUniformIDCreator>();
}

MaterialCompiler::~MaterialCompiler() = default;

MaterialCompiler& MaterialCompiler::BeginMaterial(IMaterialConfiguration* configuration)
{
    assert(!m_materialStates.has_value());

    m_materialStates = MaterialStates{};
    m_materialStates.value().pipeline = m_graphicsAPI->CreatePipeline();
    m_materialStates.value().bindLayout = m_graphicsAPI->CreateShaderBindingSetLayout();
    m_materialStates.value().configuration = configuration;

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

    m_materialStates.value().configuration->Configure(m_materialStates.value().pipeline);

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

MaterialCompiler& MaterialCompiler::SetTexture(const IMaterialConfiguration& configuration,
                                               const MaterialInstance::Textures::value_type& uniform)
{
    assert(m_materialInstanceStates.has_value());

    const auto& materialTexture = uniform.second.texture;
    auto binding = uniform.second.binding;

    if (auto imageTexture = dynamic_cast<const ImageTexture*>(materialTexture.get()); imageTexture != nullptr) {
        bindTexture(binding, uniform.first, imageTexture);
    }
    if (auto renderTexture = dynamic_cast<const RenderTexture*>(materialTexture.get()); renderTexture != nullptr) {
        bindTexture(configuration, binding, uniform.first, renderTexture);
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
        auto uniformID = !uniform.identifier.empty()
                             ? uniform.identifier
                             : m_uniformIDCreator->GetUniformIdentifier(materialID, instanceID, name);
        generalUniformMemory.SetUniformMemory(uniformID, ToBytes(uniform.value));
    }
}

std::string MaterialCompiler::MakeUniformIdentifier(uint16_t materialID, uint32_t instanceID, std::string_view name)
{
    return m_uniformIDCreator->GetUniformIdentifier(materialID, instanceID, name);
}

void MaterialCompiler::UpdateUniformMemory(std::string_view identifier, const MaterialInstance::UniformValue& value)
{
    auto& generalUniformMemory = m_resourceManager->GetMaterialGeneralUniforms();
    generalUniformMemory.SetUniformMemory(identifier, ToBytes(value));
}

void MaterialCompiler::Apply(const IMaterialConfiguration& configuration,
                             uint16_t materialID,
                             uint32_t instanceID,
                             MaterialInstance::Textures& textures)
{
    auto bindingSet = m_resourceManager->GetShaderBindingSet(materialID, instanceID);
    auto& texturesMap = m_resourceManager->GetMaterialTextures();

    for (auto& [name, texture] : textures) {
        auto& materialTexture = texture.texture;
        auto binding = texture.binding;
        auto dirty = texture.dirty;

        // Cache the identifier + kind once so subsequent frames skip std::format and dynamic_cast.
        if (texture.identifier.empty()) {
            texture.identifier = m_uniformIDCreator->GetUniformIdentifier(materialID, instanceID, name);
        }
        if (texture.kind == MaterialInstance::ETextureKind::Unknown) {
            if (dynamic_cast<const ImageTexture*>(materialTexture.get()) != nullptr) {
                texture.kind = MaterialInstance::ETextureKind::Image;
            } else if (dynamic_cast<const RenderTexture*>(materialTexture.get()) != nullptr) {
                texture.kind = MaterialInstance::ETextureKind::Render;
            }
        }
        const auto& uniformID = texture.identifier;

        if (texture.kind == MaterialInstance::ETextureKind::Image && dirty) {
            auto imageTexture = static_cast<ImageTexture*>(materialTexture.get());
            if (texturesMap.GetTexture(uniformID) == nullptr) {
                auto sampledTexture = texturesMap.AllocateTexture(uniformID, *imageTexture);
                bindingSet.BindSampledTexture(binding, sampledTexture->texture, sampledTexture->sampler);
            }
            texturesMap.SetTextureData(uniformID, imageTexture->GetImage());
            continue;
        }

        if (texture.kind == MaterialInstance::ETextureKind::Render) {
            auto renderTexture = static_cast<RenderTexture*>(materialTexture.get());
            auto bindingTexture = texturesMap.GetTexture(uniformID);
            if (bindingTexture == nullptr) {
                continue;
            }
            auto slot = configuration.GetSlot(binding);
            if (!slot.has_value()) {
                continue;
            }
            auto inputTexure = getInput(slot.value());
            if (!inputTexure.IsValid()) {
                continue;
            }
            if (!bindingTexture->texture.IsValid() || bindingTexture->texture != inputTexure) {
                bindingTexture->texture = inputTexure;
                bindingSet.BindSampledTexture(binding, inputTexure, bindingTexture->sampler);
            }
        }
    }
}

void MaterialCompiler::collectShaderbinding(std::map<uint32_t, ShaderBinding>& bindings, ShaderBinding binding)
{
    auto itr = bindings.find(binding.GetBinding());
    if (itr != bindings.end()) {
        if (itr->second == binding) {
            itr->second.EnableShaderStages(binding.GetShaderStages());
        } else {
            CS::LogError(::CS::BuiltInChannels::Render(),
                         CS::Fmt("collectShaderbinding: conflicting binding {}", binding.GetBinding()));
        }
    } else {
        bindings.emplace(binding.GetBinding(), std::move(binding));
    }
}

void MaterialCompiler::bindTexture(uint32_t binding, std::string_view uniformName, const ImageTexture* texture)
{
    auto [materialID, instanceID, bindingSet] = m_materialInstanceStates.value();
    auto uniformID = m_uniformIDCreator->GetUniformIdentifier(materialID, instanceID, uniformName);

    auto& textureMap = m_resourceManager->GetMaterialTextures();

    auto sampledTexture = textureMap.AllocateTexture(uniformID, *texture);
    bindingSet.BindSampledTexture(binding, sampledTexture->texture, sampledTexture->sampler);
}

void MaterialCompiler::bindTexture(const IMaterialConfiguration& configuration,
                                   uint32_t binding,
                                   std::string_view uniformName,
                                   const RenderTexture* texture)
{
    auto [materialID, instanceID, bindingSet] = m_materialInstanceStates.value();

    auto uniformID = m_uniformIDCreator->GetUniformIdentifier(materialID, instanceID, uniformName);
    auto& textureMap = m_resourceManager->GetMaterialTextures();

    for (const auto& slot : configuration.GetRequisiteSlots()) {
        if (!slot.binding.has_value() || slot.binding.value() != binding) {
            continue;
        }
        auto graphicTexture = getInput(slot.id);
        if (!graphicTexture.IsValid()) {
            continue;
        }
        auto sampledTexture = textureMap.AllocateTexture(uniformID, *texture, graphicTexture);
        bindingSet.BindSampledTexture(binding, sampledTexture->texture, sampledTexture->sampler);
    }
}

Texture MaterialCompiler::getInput(uint32_t slot) const
{
    auto result = std::ranges::find_if(m_currentViewInputs, [&](auto& node) { return node.first == slot; });
    return result == std::ranges::end(m_currentViewInputs) ? Texture() : result->second;
}

} // namespace CS