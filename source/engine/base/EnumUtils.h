#pragma once
#include "TypeTraits.h"

namespace CS
{

template <type_traits::enum_type EnumType>
constexpr auto EnumValue(EnumType e)
{
    return static_cast<std::underlying_type_t<EnumType>>(e);
}

} // namespace CS
