#pragma once
#include "base/EnumUtils.h"
#include <cstdint>
#include <type_traits>

namespace CS
{

enum class ShaderStage : uint16_t
{
    Vertex = 1 << 0,
    TessellationControl = 1 << 1,
    TessellationEvaluation = 1 << 2,
    Fragment = 1 << 3,
    Geometry = 1 << 4,
    Compute = 1 << 5,
    Max
};

using ShaderStageFlags = std::underlying_type_t<ShaderStage>;

constexpr inline ShaderStageFlags operator|(ShaderStage lhs, ShaderStage rhs)
{
    return EnumValue(lhs) | EnumValue(rhs);
}

constexpr inline ShaderStageFlags& operator|=(ShaderStageFlags& lhs, ShaderStage rhs)
{
    lhs |= EnumValue(rhs);
    return lhs;
}

constexpr inline ShaderStageFlags operator|(ShaderStageFlags lhs, ShaderStage rhs)
{
    return lhs | EnumValue(rhs);
}

constexpr inline ShaderStageFlags operator&(ShaderStage lhs, ShaderStage rhs)
{
    return EnumValue(lhs) & EnumValue(rhs);
}

constexpr inline ShaderStageFlags& operator&=(ShaderStageFlags& lhs, ShaderStage rhs)
{
    lhs &= EnumValue(rhs);
    return lhs;
}

constexpr inline ShaderStageFlags operator&(ShaderStageFlags lhs, ShaderStage rhs)
{
    return lhs & EnumValue(rhs);
}

static constexpr inline ShaderStageFlags AllShaderStages = ShaderStage::Vertex | ShaderStage::TessellationControl |
                                                           ShaderStage::TessellationEvaluation | ShaderStage::Fragment |
                                                           ShaderStage::Geometry | ShaderStage::Compute;

} // namespace CS
