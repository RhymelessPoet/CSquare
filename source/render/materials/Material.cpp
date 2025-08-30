#include "Material.h"
#include "MaterialComputer.h"
#include "Shader.h"

namespace CS
{
size_t Material::Builder::ID = 0u;

void Material::Compute(MaterialComputer& computer) const
{
    // clang-format off
    computer.Begin()
            .SetAttributes(m_attributes)
            .SetShaderStages(m_shaders)
            .End(m_id);
    // clang-format on
}

Material::Builder& Material::Builder::Begin()
{
    auto material = new Material();
    m_material = std::shared_ptr<Material>(material);
    m_material->m_id = ++ID;
    return *this;
}

Material::Builder& Material::Builder::AddInputAttribute(uint32_t location, VertexInputFormat format)
{
    m_material->m_attributes.emplace(location, format);
    return *this;
}

Material::Builder& Material::Builder::AddShader(std::shared_ptr<Shader> shader)
{
    m_material->m_shaders.emplace_back(std::move(shader));
    return *this;
}

std::shared_ptr<Material> Material::Builder::End()
{
    return m_material;
}

} // namespace CS
