#include "BuiltInMaterials.h"
#include "asset/BuiltInShaders.h"
#include "materials/Material.h"
#include "materials/PBRConfiguration.h"
#include <iostream>

namespace CS
{
std::shared_ptr<Material> BuiltInMaterials::GetPBRMaterial()
{
    if (m_pbrMaterial == nullptr) {
        m_pbrMaterial = createPBRMaterial();
        initializePBRMaterial();
    }

    return m_pbrMaterial;
}

std::shared_ptr<Material> BuiltInMaterials::createPBRMaterial()
{
    // clang-format off
    auto material = Material::Builder()
                    .Begin(std::make_unique<PBRConfiguration>())
                    .AddInputAttribute(0u, VertexInputFormat::Float3)
                    .AddInputAttribute(1u, VertexInputFormat::Float3)
                    .AddInputAttribute(2u, VertexInputFormat::Float3)
                    .AddInputAttribute(3u, VertexInputFormat::Float3)
                    .AddInputAttribute(4u, VertexInputFormat::Float2)
                    .AddShader(BuiltInShaders::Instance().GetVertexShader("PBR_VS"))
                    .AddShader(BuiltInShaders::Instance().GetFragmentShader("PBR_FS"))
                    .End();
    // clang-format on
    return material;
}

void BuiltInMaterials::initializePBRMaterial()
{
    bool noError{true};
    noError = noError && m_pbrMaterial->SetUniformValue("base_color", Vector4f{1.0f, 1.0f, 1.0f, 1.0f});
    noError = noError && m_pbrMaterial->SetUniformValue("diffuse_color", Vector4f{1.0f, 1.0f, 1.0f, 1.0f});
    noError = noError && m_pbrMaterial->SetUniformValue("specular_color", Vector4f{1.0f, 1.0f, 1.0f, 1.0f});
    noError = noError && m_pbrMaterial->SetUniformValue("emission_color", Vector4f{1.0f, 1.0f, 1.0f, 1.0f});
    noError = noError && m_pbrMaterial->SetUniformValue("metallic", 0.0f);
    noError = noError && m_pbrMaterial->SetUniformValue("roughness", 1.0f);
    noError = noError && m_pbrMaterial->SetUniformValue("glossiness", 0.0f);
    noError = noError && m_pbrMaterial->SetUniformValue("normal_scale", 1.0f);
    noError = noError && m_pbrMaterial->SetUniformValue("use_spec_gloss", false);
    noError = noError && m_pbrMaterial->SetUniformValue("use_emission_color_map", false);
    noError = noError && m_pbrMaterial->SetUniformValue("use_base_color_map", false);
    noError = noError && m_pbrMaterial->SetUniformValue("use_metallic_map", false);
    noError = noError && m_pbrMaterial->SetUniformValue("use_roughness_map", false);
    noError = noError && m_pbrMaterial->SetUniformValue("use_specular_map", false);
    noError = noError && m_pbrMaterial->SetUniformValue("use_glossiness_map", false);
    noError = noError && m_pbrMaterial->SetUniformValue("use_normal_map", false);

    if (!noError) {
        std::cerr << "Failed to initialize PBR material default values.\n";
    }
}

std::shared_ptr<Material> BuiltInMaterials::createPCSSShadowMaterial()
{
    return std::shared_ptr<Material>();
}

} // namespace CS
