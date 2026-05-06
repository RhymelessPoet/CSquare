#include "MaterialInstance.h"
#include "Material.h"
#include "base/Logger.h"
#include "graphics/MaterialTexture.h"
#include "renderer/MaterialCompiler.h"

namespace CS
{
EViewType MaterialInstance::GetViewType() const
{
    if (const auto mat = material()) {
        return mat->GetConfiguration().GetViewType();
    }
    return EViewType();
}

std::shared_ptr<MaterialInstance> MaterialInstance::GetRequisiteMaterial(EViewType viewType) const
{
    for (const auto& requisiteInstance : m_requisiteMaterials) {
        if (requisiteInstance->GetViewType() == viewType) {
            return requisiteInstance;
        }
        auto nestedRequisite = requisiteInstance->GetRequisiteMaterial(viewType);
        if (nestedRequisite != nullptr) {
            return nestedRequisite;
        }
    }
    return nullptr;
}

std::shared_ptr<MaterialInstance> MaterialInstance::Clone() const
{
    auto clonedInstance = material()->CreateInstance();
    clonedInstance->m_uniforms = m_uniforms;
    for (const auto& [name, textureUniform] : m_textures) {
        clonedInstance->m_textures.emplace(
            name, TextureUniform{textureUniform.texture->Clone(), textureUniform.binding, textureUniform.dirty});
    }
    for (const auto& requisiteInstance : m_requisiteMaterials) {
        clonedInstance->m_requisiteMaterials.push_back(requisiteInstance->Clone());
    }
    clonedInstance->m_name = m_name;
    return clonedInstance;
}

MaterialInstance::MaterialInstance(std::weak_ptr<Material> material, Uniforms uniforms, Textures textures)
    : m_material(std::move(material)), m_uniforms(std::move(uniforms)), m_textures(std::move(textures))
{}

MaterialInstance::MaterialInstance(std::shared_ptr<Material> material, uint32_t id)
    : m_material(std::move(material)), m_id(id)
{}

bool MaterialInstance::SetTexture(std::string_view name, std::unique_ptr<MaterialTexture> texture)
{
    auto itr = m_textures.find(std::string(name));

    if (itr != m_textures.end()) {
        if (itr->second.texture != texture) {
            itr->second.texture = std::move(texture);
            itr->second.dirty = true;
        }
        return true;
    } else if (m_id == 0u) {
        return false;
    } else {
        auto defaultTexture = GetDefaultInstance().getTextureUniform(name);
        if (defaultTexture != nullptr) {
            m_textures.emplace(name, TextureUniform{std::move(texture), defaultTexture->binding, true});
            return true;
        } else {
            return false;
        }
    }
    return false;
}

const MaterialTexture* MaterialInstance::GetTexture(std::string_view name) const
{
    auto itr = m_textures.find(std::string(name));

    if (itr != m_textures.end()) {
        return itr->second.texture.get();
    }
    if (m_id != 0u) {
        return GetDefaultInstance().GetTexture(name);
    }
    return nullptr;
}

MaterialTexture* MaterialInstance::GetInstanceTexture(std::string_view name)
{
    auto itr = m_textures.find(std::string(name));

    if (itr != m_textures.end()) {
        itr->second.dirty = true;
        return itr->second.texture.get();
    }
    if (m_id != 0u) {
        return GetDefaultInstance().GetInstanceTexture(name);
    }
    return nullptr;
}

void MaterialInstance::Compile(MaterialCompiler& compiler) const
{
    auto bindingSetLayout = compiler.GetBindingSetLayout(*material());
    compiler.BeginMaterialInstance(GetMaterialID(), m_id, bindingSetLayout);

    for (const auto& binding : instancedUniformBindings()) {
        compiler.SetUniformBuffer(binding, getUniforms(binding));
    }

    for (const auto& pair : m_textures) {
        compiler.SetTexture(material()->GetConfiguration(), pair);
    }

    compiler.EndMaterialInstance();
}

void MaterialInstance::Apply(MaterialCompiler& compiler)
{
    auto materialID = GetMaterialID();
    for (const auto& binding : instancedUniformBindings()) {
        auto uniforms = getUniforms(binding);
        compiler.Apply(materialID, m_id, uniforms);

        for (const auto& [name, _] : uniforms) {
            auto uniform = getUniform(name);
            if (uniform == nullptr) {
                CS::LogError(::CS::BuiltInChannels::Render(), CS::Fmt("Uniform '{}' not found during Apply", name));
                continue;
            }
            uniform->dirty = false;
        }
    }

    compiler.Apply(material()->GetConfiguration(), materialID, m_id, m_textures);
    for (auto& [_, texture] : m_textures) {
        texture.dirty = false;
    }
}

uint16_t MaterialInstance::GetMaterialID() const
{
    return material()->GetID();
}

bool MaterialInstance::HasInstancedUniform() const
{
    if (m_id == 0u) {
        return true;
    } else {
        return !m_uniforms.empty();
    }
}

const MaterialInstance& MaterialInstance::GetDefaultInstance() const
{
    return material()->GetDefaultInstance();
}

MaterialInstance& MaterialInstance::GetDefaultInstance()
{
    return material()->GetDefaultInstance();
}

std::shared_ptr<Material> MaterialInstance::GetMaterial()
{
    return material();
}

std::shared_ptr<Material> MaterialInstance::material() const
{
    auto materialSPtr = [](auto&& arg) -> std::shared_ptr<Material> {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (type_traits::weak_pointer<T>) {
            return arg.lock();
        } else {
            return arg;
        }
    };

    auto _material = std::visit(materialSPtr, m_material);
    assert(_material != nullptr);

    return _material;
}

std::shared_ptr<Material> MaterialInstance::material()
{
    return std::as_const(*this).material();
}

std::set<uint32_t> MaterialInstance::instancedUniformBindings() const
{
    std::set<uint32_t> bindings;

    for (const auto& [name, uniform] : m_uniforms) {
        bindings.insert(uniform.binding);
    }

    return bindings;
}

MaterialInstance::Uniforms MaterialInstance::getUniforms(uint32_t binding) const
{
    if (m_id == 0u) {
        return getSelfUniforms(binding);
    }
    auto uniforms = material()->GetDefaultInstance().getSelfUniforms(binding);
    instanceUniforms(uniforms, binding);
    return uniforms;
}

MaterialInstance::Uniforms MaterialInstance::getSelfUniforms(uint32_t binding) const
{
    Uniforms result;
    for (const auto& [name, uniform] : m_uniforms) {
        if (uniform.binding == binding) {
            result[name] = uniform;
        }
    }
    return result;
}

void MaterialInstance::instanceUniforms(Uniforms& uniforms, uint32_t binding) const
{
    auto selfUniforms = getSelfUniforms(binding);
    for (const auto& [name, uniform] : selfUniforms) {
        uniforms[name] = uniform;
    }
}

MaterialInstance::Uniform* MaterialInstance::getUniform(std::string_view name)
{
    auto itr = m_uniforms.find(std::string(name));
    if (itr != m_uniforms.end()) {
        return &itr->second;
    }
    return nullptr;
}

const MaterialInstance::TextureUniform* MaterialInstance::getTextureUniform(std::string_view name) const
{
    auto itr = m_textures.find(std::string(name));
    if (itr != m_textures.end()) {
        return &itr->second;
    }
    return nullptr;
}

uint32_t SizeOf(const MaterialInstance::UniformValue& value)
{
    return ByteSizeOf(value) * 8u;
}

uint32_t ByteSizeOf(const MaterialInstance::UniformValue& value)
{
    auto size_of = [](auto&& arg) {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (type_traits::smart_pointer<T>) {
            return sizeof(std::shared_ptr<void>);
        } else if constexpr (type_traits::is_vector_v<T>) {
            return arg.size() * sizeof(T::value_type);
        } else {
            return sizeof(T);
        }
    };
    return std::visit(size_of, value);
}

std::vector<std::byte> ToBytes(const MaterialInstance::UniformValue& value)
{
    auto address = [=](auto&& arg) {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (type_traits::smart_pointer<T>) {
            return reinterpret_cast<const std::byte*>(arg.get());
        } else if constexpr (type_traits::is_vector_v<T>) {
            return reinterpret_cast<const std::byte*>(arg.data());
        } else {
            return reinterpret_cast<const std::byte*>(std::addressof(arg));
        }
    };
    auto data = std::visit(address, value);
    return std::vector<std::byte>(data, data + ByteSizeOf(value));
}

} // namespace CS