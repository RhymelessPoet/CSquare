#pragma once
#include <cstdint>

namespace CS
{

enum class TextureFormat : uint8_t
{
    RGBA8Unorm = 0u,
    RGB8Unorm,
    BGRA8Unorm,
    RGBA8Srgb,
    RGB32Float,
    RGBA32Float,
    Depth24Stencil8,
    Max
};

enum class AddressMode : uint8_t
{
    Repeat = 0u,
    MirroredRepeat,
    ClampToEdge,
    ClampToBorder,
    Max
};

enum class FilterMode : uint8_t
{
    Nearest = 0u,
    Linear,
    Max
};

enum class MipmapFilterMode : uint8_t
{
    Nearest = 0u,
    Linear,
    Max
};

enum class DepthCompareOp : uint8_t
{
    Never = 0u,
    Less,
    Equal,
    LessEqual,
    Greater,
    NotEqual,
    GreaterEqual,
    Always,
    Max
};

} // namespace CS
