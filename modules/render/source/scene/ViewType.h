#pragma once
#include "ExtensibleEnum.h"

namespace CS
{

struct ViewType
{
    using UnderlyingType = uint8_t;
    static constexpr size_t NameLength{16u};
    static constexpr size_t MaxCount = std::numeric_limits<UnderlyingType>::max() + 1;
};

const inline EnumClass<ViewType, "3D_Main", "Shadow_Map"> PreDefineViewTypes;

using EViewType = Enum<ViewType>;

} // namespace CS