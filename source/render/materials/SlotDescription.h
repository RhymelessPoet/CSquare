#pragma once
#include "base/Size.h"
#include <optional>

namespace CS
{

class IMaterialConfiguration;

enum class SlotType : uint8_t
{
    Color = 0,
    Depth,
    Stencil,
    DepthStencil,
    Max
};

struct SlotDescription
{
    struct ID
    {
        uint16_t materialID{0u};
        uint16_t slotIndex{0u};

        constexpr operator uint32_t() const
        {
            return (static_cast<uint32_t>(materialID) << 16) | static_cast<uint32_t>(slotIndex);
        }
    };

    ID id;
    std::optional<Size3u> size;
    std::optional<ID> connectedSlot;
    std::optional<uint32_t> binding;
    SlotType type{SlotType::Color};
};

using SlotID = SlotDescription::ID;

} // namespace CS
