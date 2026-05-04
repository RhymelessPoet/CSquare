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
    for (auto material : m_requisiteMaterials) {
        auto requisiteInstance = material->CreateInstance();
        instance->m_requisiteMaterials.push_back(std::move(requisiteInstance));
    }
    return std::shared_ptr<MaterialInstance>(instance);
}

void Material::Compile(MaterialCompiler& compiler) const
{
    // clang-format off
    compiler.BeginMaterial(m_configuration.get())
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

Material::Builder& Material::Builder::Begin(std::unique_ptr<IMaterialConfiguration> configuration)
{
    auto material = new Material();
    m_material = std::shared_ptr<Material>(material);
    m_material->m_configuration = std::move(configuration);

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
            for (const auto& property : binding.GetLayout()) {
                insertUniform(bindingNum, property);
            }
        }
    }
    m_material->m_shaders.emplace_back(std::move(shader));
    return *this;
}

Material::Builder& Material::Builder::AddRequisiteMaterial(std::shared_ptr<Material> material)
{
    m_material->m_requisiteMaterials.push_back(std::move(material));
    return *this;
}

Material::Builder& Material::Builder::Connect(const SlotID& from, uint16_t toSlotIndex)
{
    if (!m_connects.emplace(from, toSlotIndex).second) {
        // TODO: log warning
    }
    return *this;
}

Material::Builder& Material::Builder::Bind(uint16_t slotIndex, uint32_t binding)
{
    if (!m_bindings.emplace(slotIndex, binding).second) {
        // TODO: log warning
    }
    return *this;
}

std::shared_ptr<Material> Material::Builder::End()
{
    m_material->createDefaultInstance(std::move(m_uniforms), std::move(m_textures));
    m_material->m_id = ++ID;

    auto& configuration = m_material->GetConfiguration();
    configuration.SetMaterialID(ID);

    for (const auto& [from, to] : m_connects) {
        if (!connectFrom(from, {ID, to})) {
            // TODO: log warning
        }
        if (!connectTo(from, {ID, to})) {
            disconnectFrom(from);
            // TODO: log warning
        }
    }

    for (auto [slotIndex, binding] : m_bindings) {
        if (!configuration.Bind({ID, slotIndex}, binding)) {
            // TODO: log warning
        }
    }

    return m_material;
}

void Material::Builder::insertUniform(uint32_t binding, const ShaderBindingProperty& property)
{
    if (auto itr = m_uniforms.find(property.name); itr != m_uniforms.end()) {
        auto& [value, _binding, _offset, _] = itr->second;
        if (_binding != binding || _offset != property.offset) {
            // TODO: log error
        }
    } else {
        m_uniforms.emplace(property.name, MaterialInstance::Uniform{.binding = binding, .offset = property.offset});
    }
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
bool Material::Builder::connectFrom(const SlotID& from, const SlotID& to)
{
    for (const auto& requisiteMaterial : m_material->m_requisiteMaterials) {
        if (requisiteMaterial->GetConfiguration().ConnectFrom(from, to)) {
            return true;
        }
    }
    return false;
}
bool Material::Builder::connectTo(const SlotID& from, const SlotID& to)
{
    return m_material->GetConfiguration().ConnectTo(from, to);
}
void Material::Builder::disconnectFrom(const SlotID& id)
{
    for (const auto& requisiteMaterial : m_material->m_requisiteMaterials) {
        if (requisiteMaterial->GetConfiguration().DisconnectFrom(id)) {
            break;
        }
    }
}
} // namespace CS
