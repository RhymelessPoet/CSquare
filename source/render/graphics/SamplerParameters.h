#pragma once
#include <cstdint>

namespace CS
{

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

} // namespace CS
