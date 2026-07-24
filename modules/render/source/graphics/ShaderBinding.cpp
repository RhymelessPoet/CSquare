#include "ShaderBinding.h"
#include "MaterialTexture.h"
#include <cassert>

namespace CS
{
ShaderBindingTexture::ShaderBindingTexture(std::string_view _name, std::unique_ptr<MaterialTexture> _texture)
    : name(_name), texture(std::move(_texture))
{}

ShaderBindingTexture::ShaderBindingTexture(const ShaderBindingTexture& other)
    : name(other.name), texture(other.texture ? other.texture->Clone() : nullptr)
{}

ShaderBindingTexture& ShaderBindingTexture::operator=(const ShaderBindingTexture& other)
{
    if (this != &other) {
        name = other.name;
        texture = other.texture ? other.texture->Clone() : nullptr;
    }
    return *this;
}

ShaderBinding::ShaderBinding(uint32_t binding, ShaderStageFlags stages, Type type)
    : m_binding(binding), m_stages(stages), m_type(type)
{}

ShaderBinding::ShaderBinding(uint32_t binding, ShaderStage stage, Type type)
    : m_binding(binding), m_stages(EnumValue(stage)), m_type(type)
{}

size_t ShaderBinding::GetSize() const
{
    if (m_type != Type::UniformBuffer) {
        return 0u;
    }

    const auto& layout = GetLayout();
    if (layout.empty()) {
        return 0u;
    }
    return layout.back().offset + layout.back().size;
}

void ShaderBinding::SetLayout(std::vector<ShaderBindingProperty> properties)
{
    m_data = std::move(properties);
    m_type = Type::UniformBuffer;

    uint32_t offset = 0u;
    for (auto& property : std::get<std::vector<ShaderBindingProperty>>(m_data)) {
        property.offset = std140Align(offset, property.size);
        offset = property.offset + property.size;
    }
}

const std::vector<ShaderBindingProperty>& ShaderBinding::GetLayout() const
{
    assert(std::holds_alternative<std::vector<ShaderBindingProperty>>(m_data));
    return std::get<std::vector<ShaderBindingProperty>>(m_data);
}

void ShaderBinding::SetTexture(ShaderBindingTexture&& texture)
{
    m_data = std::move(texture);
    m_type = Type::SampledTexture;
}

const ShaderBindingTexture& ShaderBinding::GetTexture() const
{
    assert(std::holds_alternative<ShaderBindingTexture>(m_data));
    return std::get<ShaderBindingTexture>(m_data);
}

inline uint32_t ShaderBinding::std140Align(uint32_t baseOffset, uint32_t propertySize)
{
    // 根据属性大小，获取std140规则下的「基本对齐值」
    auto getStd140BaseAlignment = [](uint32_t size) {
        // std140基本对齐规则映射（覆盖常见类型）
        if (size == 1) { // bool (std140中占4字节，此处兼容实际存储)
            return 4u;
        } else if (size == 2u) { // 未直接定义的2字节类型（如short），std140对齐4
            return 4u;
        } else if (size == 4u) { // float/int/uint/bool（实际占4字节）
            return 4u;
        } else if (size == 8u) { // vec2/dvec1(double)/ivec2/uvec2
            return 8u;
        } else if (size == 12u) { // vec3（大小12，对齐16）
            return 16u;
        } else if (size == 16u) { // vec4/mat2/dvec2/ivec4/uvec4
            return 16u;
        } else if (size == 24u) { // dvec3（大小24，对齐32）
            return 32u;
        } else if (size == 32u) { // dvec4/mat4（double版）
            return 32u;
        } else {
            // 其他大小：取大于等于propertySize的最小2的幂（兼容自定义结构体/数组）
            uint32_t align = 1u;
            while (align < size) {
                align <<= 1u;
            }
            // std140数组步长至少16（若对齐<16）
            return std::max(align, 16u);
        }
    };

    auto baseAlign = getStd140BaseAlignment(propertySize);

    // 计算对齐后的偏移
    uint32_t alignedOffset = baseOffset;
    if (alignedOffset % baseAlign != 0u) {
        alignedOffset += baseAlign - (alignedOffset % baseAlign);
    }
    return alignedOffset;
}

} // namespace CS
