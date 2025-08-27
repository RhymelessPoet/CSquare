#include "VertexInputLayout.h"

namespace CS
{

VertexInputAttribute::Size VertexInputAttribute::GetSize() const
{
    switch (m_format) {
    case Format::Float:
        return Size{sizeof(float), 1u};
    case Format::Float2:
        return Size{sizeof(float), 2u};
    case Format::Float3:
        return Size{sizeof(float), 3u};
    case Format::Float4:
        return Size{sizeof(float), 4u};
    case Format::Int:
        return Size{sizeof(int), 1u};
    case Format::Int2:
        return Size{sizeof(int), 2u};
    case Format::Int3:
        return Size{sizeof(int), 3u};
    case Format::Int4:
        return Size{sizeof(int), 4u};
    case Format::UInt:
        return Size{sizeof(unsigned int), 1u};
    case Format::UInt2:
        return Size{sizeof(unsigned int), 2u};
    case Format::UInt3:
        return Size{sizeof(unsigned int), 3u};
    case Format::UInt4:
        return Size{sizeof(unsigned int), 4u};
    default:
        return Size{}; // Invalid format
    }
}

VertexInputLayout& VertexInputLayout::SetBinding(uint32_t binding, const VertexInputBinding& inputBinding)
{
    if (binding < MaxBindings) {
        m_bindings[binding] = inputBinding;
    }
    return *this;
}

const std::optional<VertexInputBinding>& VertexInputLayout::GetBinding(uint32_t binding) const
{
    binding = std::min(binding, MaxBindings);
    return m_bindings[binding];
}

VertexInputLayout& VertexInputLayout::SetAttribute(uint32_t location, const VertexInputAttribute& attribute)
{
    if (location < MaxAttributes) {
        m_attributes[location] = attribute;
    }

    return *this;
}

const std::optional<VertexInputAttribute>& VertexInputLayout::GetAttribute(uint32_t location) const
{
    location = std::min(location, MaxAttributes);
    return m_attributes[location];
}

} // namespace CS
