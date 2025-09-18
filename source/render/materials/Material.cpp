#include "Material.h"
#include "Shader.h"
#include "base/TypeTraits.h"
#include "graphics/MaterialTexture.h"
#include "renderer/MaterialCompiler.h"

namespace CS
{
uint16_t Material::Builder::ID = 0u;

std::shared_ptr<MaterialInstance> Material::CreateInstance()
{
    auto instance = new MaterialInstance(shared_from_this(), ++m_instanceID);
    return std::shared_ptr<MaterialInstance>(instance);
}

void Material::Compile(MaterialCompiler& compiler) const
{
    // clang-format off
    compiler.BeginMaterial()
            .SetAttributes(m_attributes)
            .SetShaderStages(m_shaders)
            .EndMaterial(m_id);
    // clang-format on
}

void Material::createDefaultInstance(MaterialInstance::Uniforms uniforms, MaterialInstance::Textures textures)
{
    auto instance = new MaterialInstance(weak_from_this(), std::move(uniforms), std::move(textures));
    m_defaultInstance.reset(instance);
}

Material::Builder& Material::Builder::Begin()
{
    auto material = new Material();
    m_material = std::shared_ptr<Material>(material);
    return *this;
}

Material::Builder& Material::Builder::AddInputAttribute(uint32_t location, VertexInputFormat format)
{
    m_material->m_attributes.emplace(location, format);
    return *this;
}

Material::Builder& Material::Builder::AddShader(std::shared_ptr<Shader> shader)
{
    for (const auto& binding : shader->GetBindings()) {
        auto bindingNum = binding.GetBinding();
        if (binding.GetType() == ShaderBinding::Type::SampledTexture) {
            insertUniformTexture(bindingNum, binding.GetTexture());
        } else if (binding.GetType() == ShaderBinding::Type::UniformBuffer) {
            uint32_t offset = 0u;
            for (const auto& property : binding.GetLayout()) {
                offset = insertUniform(bindingNum, offset, property);
            }
        }
    }
    m_material->m_shaders.emplace_back(std::move(shader));
    return *this;
}

std::shared_ptr<Material> Material::Builder::End()
{
    m_material->createDefaultInstance(std::move(m_uniforms));
    m_material->m_id = ++ID;

    return m_material;
}

uint32_t Material::Builder::insertUniform(uint32_t binding, uint32_t offset, const ShaderBindingProperty& property)
{
    auto itr = m_uniforms.find(property.name);

    if (itr != m_uniforms.end()) {
        auto& [value, _binding, _offset, _] = itr->second;
        if (_binding == binding && _offset == offset + property.size) {
            offset = _offset;
        } else {
            // TODO: log error
        }
    } else {
        m_uniforms.emplace(property.name, MaterialInstance::Uniform{.binding = binding, .offset = offset});
        offset += property.size;
    }
    return offset;
}

void Material::Builder::insertUniformTexture(uint32_t binding, const ShaderBindingTexture& texture)
{
    auto itr = m_textures.find(texture.name);
    if (itr == m_textures.end()) {
        m_textures.emplace(texture.name, MaterialInstance::TextureUniform{
                                             .texture = texture.texture->Clone(), .binding = binding, .dirty = true});
    } else {
        if (itr->second.binding != binding || itr->second.texture != texture.texture) {
            // TODO: log error
        }
    }
}

} // namespace CS
