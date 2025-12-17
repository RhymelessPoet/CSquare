#include "BuiltInMaterials.h"
#include "asset/BuiltInShaders.h"
#include "materials/Material.h"

namespace CS
{
std::shared_ptr<Material> BuiltInMaterials::GetPBRMaterial()
{
    if (m_pbrMaterial == nullptr) {
        m_pbrMaterial = createPBRMaterial();
    }

    return m_pbrMaterial;
}

std::shared_ptr<Material> BuiltInMaterials::createPBRMaterial()
{
    // clang-format off
    auto material = Material::Builder()
                    .Begin()
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

} // namespace CS
