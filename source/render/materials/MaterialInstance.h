#pragma once
#include "base/TypeTraits.h"
#include "base/math/Vector.h"
#include "graphics/MaterialTexture.h"

#include <map>
#include <memory>
#include <set>
#include <string>
#include <variant>
#include <vector>

namespace CS
{
class Material;
class MaterialCompiler;
class Image;
class MaterialTexture;

class MaterialInstance
{
public:
    friend class Material;
    using UniformValue = std::variant<bool, int, uint32_t, float, Vector2f, Vector3f, Vector4f, std::vector<float>>;
    struct Uniform
    {
        UniformValue value;
        uint32_t binding{0};
        uint32_t offset{0};
        bool dirty{true};
    };

    struct TextureUniform
    {
        std::unique_ptr<MaterialTexture> texture;
        uint32_t binding;
        bool dirty{true};
    };

    using Uniforms = std::map<std::string, Uniform>;
    using Textures = std::map<std::string, TextureUniform>;

    template <typename T>
        requires type_traits::is_in_variant_v<std::remove_cvref_t<T>, UniformValue>
    [[nodiscard]] bool SetUniformValue(std::string_view name, T&& value)
    {
        auto itr = m_uniforms.find(std::string(name));
        if (itr != m_uniforms.end()) {
            itr->second.value = std::forward<T>(value);
            itr->second.dirty = true;
            return true;
        } else if (m_id == 0u) {
            return false;
        } else {
            auto uniform = GetDefaultInstance().getUniform(name);
            if (uniform != nullptr) {
                m_uniforms.emplace(name, Uniform{std::forward<T>(value), uniform->binding, uniform->offset, true});
                return true;
            } else {
                return false;
            }
        }
        return false;
    }

    template <typename T>
        requires type_traits::is_in_variant_v<T, Uniform>
    [[nodiscard]] bool GetUniformValue(std::string_view name, T& outValue) const
    {
        auto itr = m_uniforms.find(name);
        if (itr != m_uniforms.end()) {
            if (auto val = std::get_if<T>(&itr->second.value); val != nullptr) {
                outValue = *val;
                return true;
            }
        }
        if (m_id != 0u) {
            return GetDefaultInstance().GetUniformValue(name, outValue);
        }
        return false;
    }

    [[nodiscard]] bool SetTexture(std::string_view name, std::unique_ptr<MaterialTexture> texture);
    const MaterialTexture* GetTexture(std::string_view name) const;
    MaterialTexture* GetInstanceTexture(std::string_view name);

    void Compile(MaterialCompiler& compiler) const;
    void Apply(MaterialCompiler& compiler);

    uint16_t GetMaterialID() const;
    uint32_t GetID() const { return m_id; }

    bool HasInstancedUniform() const;

    const MaterialInstance& GetDefaultInstance() const;
    MaterialInstance& GetDefaultInstance();

    std::shared_ptr<Material> GetMaterial();

    void SetName(std::string_view name) { m_name = name; }
    std::string_view GetName() const { return m_name; }

    std::shared_ptr<MaterialInstance> Clone() const;

private:
    MaterialInstance(std::weak_ptr<Material> material, Uniforms uniforms, Textures textures = {});
    MaterialInstance(std::shared_ptr<Material> material, uint32_t id);

    std::shared_ptr<Material> material() const;
    std::shared_ptr<Material> material();

    std::set<uint32_t> instancedUniformBindings() const;
    Uniforms getUniforms(uint32_t binding) const;
    Uniforms getSelfUniforms(uint32_t binding) const;
    void instanceUniforms(Uniforms& uniforms, uint32_t binding) const;

    Uniform* getUniform(std::string_view name);
    const TextureUniform* getTextureUniform(std::string_view name) const;

private:
    using MaterialPtr = std::variant<std::shared_ptr<Material>, std::weak_ptr<Material>>;
    uint32_t m_id{0u};
    MaterialPtr m_material;
    Uniforms m_uniforms;
    Textures m_textures;

    std::string m_name;
};

uint32_t SizeOf(const MaterialInstance::UniformValue& value);
uint32_t ByteSizeOf(const MaterialInstance::UniformValue& value);
std::vector<std::byte> ToBytes(const MaterialInstance::UniformValue& value);

} // namespace CS
