#include "VertexInputLayout.h"

namespace CS
{

VertexFormatSize VertexInputAttribute::GetSize() const
{
    return VertexInputFormatSize(m_format);
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
