#include "ShaderBinding.h"
#include "MaterialTexture.h"
#include <cassert>

namespace CS
{

ShaderBindingTexture::ShaderBindingTexture(const ShaderBindingTexture& other)
    : name(other.name), texture(other.texture ? other.texture->Clone() : nullptr)
{}

ShaderBindingTexture::~ShaderBindingTexture() = default;

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
    size_t size = 0u;
    for (const auto& property : GetLayout()) {
        size += property.size;
    }
    return size;
}

void ShaderBinding::SetLayout(std::vector<ShaderBindingProperty> properties)
{
    m_data = std::move(properties);
    m_type = Type::UniformBuffer;
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

} // namespace CS
