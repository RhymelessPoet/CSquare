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
    RGB8Srgb,
    RGB32Float,
    RGBA32Float,
    Depth24Stencil8,
    Depth32,
    Depth32Float,
    Stencil32,
    Max
};

enum class AddressMode : uint8_t
{
    Repeat = 0u,
    MirroredRepeat,
    ClampToEdge,
    ClampToBorder,
    Decal,
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

static constexpr inline size_t GetTextureFormatSize(TextureFormat format)
{
    switch (format) {
    case TextureFormat::RGBA8Unorm:
    case TextureFormat::BGRA8Unorm:
    case TextureFormat::RGBA8Srgb:
        return 4;
    case TextureFormat::RGB8Unorm:
    case TextureFormat::RGB8Srgb:
        return 3;
    case TextureFormat::RGB32Float:
        return 12;
    case TextureFormat::RGBA32Float:
        return 16;
    case TextureFormat::Depth24Stencil8:
        return 4;
    default:
        return 4;
    }
}

} // namespace CS
