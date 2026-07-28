#pragma once
#include <cstdint>
#include <type_traits>

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

enum class BlendFactor : uint8_t
{
    Zero = 0u,
    One,
    SrcColor,
    OneMinusSrcColor,
    DstColor,
    OneMinusDstColor,
    SrcAlpha,
    OneMinusSrcAlpha,
    DstAlpha,
    OneMinusDstAlpha,
    ConstantColor,
    OneMinusConstantColor,
    ConstantAlpha,
    OneMinusConstantAlpha,
    SrcAlphaSaturate,
    Max
};

enum class BlendOp : uint8_t
{
    Add = 0u,
    Subtract,
    ReverseSubtract,
    Min,
    Max_,
    Max
};

// Face culling mode. Maps to OpenGL's glCullFace(GL_FRONT | GL_BACK | GL_FRONT_AND_BACK).
// `None` disables culling entirely (glDisable(GL_CULL_FACE)).
enum class CullMode : uint8_t
{
    None = 0u,
    Front,
    Back,
    FrontAndBack,
    Max
};

enum class StorageTextureAccess : uint8_t { ReadOnly, WriteOnly, ReadWrite };

enum class MemoryBarrier : uint32_t
{
    None = 0,
    VertexBuffer = 1u << 0,
    IndexBuffer = 1u << 1,
    UniformBuffer = 1u << 2,
    StorageBuffer = 1u << 3,
    TextureFetch = 1u << 4,
    StorageTexture = 1u << 5,
    All = 0xffffffffu
};

constexpr MemoryBarrier operator|(MemoryBarrier lhs, MemoryBarrier rhs)
{
    return static_cast<MemoryBarrier>(static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs));
}

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
