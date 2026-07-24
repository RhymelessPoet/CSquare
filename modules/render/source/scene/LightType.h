#pragma once
#include "ExtensibleEnum.h"

namespace CS
{

struct LightType
{
    using UnderlyingType = uint8_t;
    static constexpr size_t NameLength{16u};
    static constexpr size_t MaxCount = std::numeric_limits<UnderlyingType>::max() + 1;
};

const inline EnumClass<LightType, "Directional", "Point", "Spot"> PreDefineLightTypes;

using ELightType = Enum<LightType>;

} // namespace CS
