#pragma once
#include "VertexInputFormat.h"
#include <array>
#include <map>
#include <optional>
#include <vector>

namespace CS
{
enum class IndexFormat
{
    UInt16 = 0u,
    UInt32,
    Max
};

class VertexInputAttribute
{
public:
    VertexInputAttribute(uint32_t binding, VertexInputFormat format, uint32_t offset)
        : m_binding(binding), m_format(format), m_offset(offset)
    {}

    uint32_t GetBinding() const { return m_binding; }
    VertexInputFormat GetFormat() const { return m_format; }
    uint32_t GetOffset() const { return m_offset; }
    VertexFormatSize GetSize() const;

private:
    uint32_t m_binding{0u};
    VertexInputFormat m_format{VertexInputFormat::Float};
    uint32_t m_offset{0u};
};

class VertexInputBinding
{
public:
    enum class InputRate
    {
        PerVertex = 0,
        PerInstance,
        Max
    };
    VertexInputBinding(uint32_t stride, InputRate inputRate) : m_stride(stride), m_inputRate(inputRate) {}

    InputRate GetInputRate() const { return m_inputRate; }
    uint32_t GetStride() const { return m_stride; }

private:
    uint32_t m_stride{0u};
    InputRate m_inputRate{InputRate::PerVertex};
};

using VertexInputRate = VertexInputBinding::InputRate;

class VertexInputLayout
{
public:
    static constexpr uint32_t MaxBindings = 8u;
    static constexpr uint32_t MaxAttributes = 8u;

public:
    VertexInputLayout(/* args */) = default;
    ~VertexInputLayout() = default;

    VertexInputLayout& SetBinding(uint32_t binding, const VertexInputBinding& inputBinding);
    const std::optional<VertexInputBinding>& GetBinding(uint32_t binding) const;

    VertexInputLayout& SetAttribute(uint32_t location, const VertexInputAttribute& attribute);
    const std::optional<VertexInputAttribute>& GetAttribute(uint32_t location) const;

private:
    std::array<std::optional<VertexInputBinding>, MaxBindings + 1u> m_bindings;
    std::array<std::optional<VertexInputAttribute>, MaxAttributes + 1u> m_attributes;
};

} // namespace CS
