#pragma once
#include <cstdint>

namespace CS
{

enum class ImageFormat : uint8_t
{
    RGB8,
    RGBA8,
    ARGB8,
    RGB32Float,
    RGBA32Float,
    Max
};

enum class ImageChannelType
{
    Byte,
    Float16,
    Float32,
    Max
};

inline uint32_t Channels(ImageFormat format)
{
    switch (format) {
    case ImageFormat::RGB8:
        return 3u;
    case ImageFormat::ARGB8:
    case ImageFormat::RGBA8:
        return 4u;
    case ImageFormat::RGB32Float:
        return 3u;
    case ImageFormat::RGBA32Float:
        return 4u;
    default:
        return 0u;
    }
}

inline ImageChannelType ChannelType(ImageFormat format)
{
    switch (format) {
    case ImageFormat::RGB8:
        return ImageChannelType::Byte;
    case ImageFormat::RGBA8:
        return ImageChannelType::Byte;
    case ImageFormat::RGB32Float:
        return ImageChannelType::Float32;
    case ImageFormat::RGBA32Float:
        return ImageChannelType::Float32;
    default:
        return ImageChannelType::Max;
    }
}

inline uint32_t ChannelByteSize(ImageChannelType type)
{
    switch (type) {
    case ImageChannelType::Byte:
        return 1u;
    case ImageChannelType::Float16:
        return 2u;
    case ImageChannelType::Float32:
        return 4u;
    default:
        return 0u;
    }
};

} // namespace CS
