#pragma once
#include "ShaderStage.h"
#include "graphics/MaterialTexture.h"
#include <optional>
#include <string>
#include <vector>

namespace CS
{
class MaterialTexture;
struct ShaderBindingProperty
{
    std::string name;
    uint32_t size;
};

struct ShaderBindingTexture
{
    ShaderBindingTexture(std::string_view name, std::unique_ptr<MaterialTexture> texture);
    ShaderBindingTexture(const ShaderBindingTexture&);
    ShaderBindingTexture& operator=(const ShaderBindingTexture&);

    std::string name;
    std::unique_ptr<MaterialTexture> texture;
};
class ShaderBinding
{
public:
    enum class Type
    {
        UniformBuffer,
        StorageBuffer,
        SampledTexture,
        StorageTexture,
        Sampler,
        AccelerationStructure,
        Max
    };

    ShaderBinding(uint32_t binding, ShaderStageFlags stages, Type type);
    ShaderBinding(uint32_t binding, ShaderStage stage, Type type);

    uint32_t GetBinding() const { return m_binding; }
    std::optional<uint32_t> GetSet() const { return m_set; }

    size_t GetSize() const;

    void EnableShaderStages(ShaderStageFlags stages) { m_stages = m_stages | stages; }
    ShaderStageFlags GetShaderStages() const { return m_stages; }
    Type GetType() const { return m_type; }

    void SetLayout(std::vector<ShaderBindingProperty> properties);
    const std::vector<ShaderBindingProperty>& GetLayout() const;

    void SetTexture(ShaderBindingTexture&& texture);
    const ShaderBindingTexture& GetTexture() const;

private:
    uint32_t m_binding{0};
    std::optional<uint32_t> m_set;
    ShaderStageFlags m_stages{EnumValue(ShaderStage::Vertex)};
    Type m_type{Type::Max};

    using BindingData = std::variant<std::monostate, std::vector<ShaderBindingProperty>, ShaderBindingTexture>;
    BindingData m_data;
};

inline bool operator==(const ShaderBinding& lhs, const ShaderBinding& rhs)
{
    if (lhs.GetBinding() != rhs.GetBinding()) {
        return false;
    }
    if (lhs.GetSet().has_value() != rhs.GetSet().has_value()) {
        return false;
    }
    if (lhs.GetSet().has_value() && rhs.GetSet().has_value() && lhs.GetSet().value() != rhs.GetSet().value()) {
        return false;
    }
    if (lhs.GetType() != rhs.GetType()) {
        return false;
    }
    const auto& lProperties = lhs.GetLayout();
    const auto& rProperties = rhs.GetLayout();

    if (lProperties.size() != rProperties.size()) {
        return false;
    }

    for (uint32_t index = 0; index < lProperties.size(); ++index) {
        auto& [lName, lSize] = lProperties[index];
        auto& [rName, rSize] = rProperties[index];

        if (lName != rName || lSize != rSize) {
            return false;
        }
    }
    return true;
}

inline bool operator!=(const ShaderBinding& lhs, const ShaderBinding& rhs)
{
    return !(lhs == rhs);
}

} // namespace CS
