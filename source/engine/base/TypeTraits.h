#pragma once
#include <type_traits>
#include <variant>

namespace CS
{

namespace type_traits
{
template <typename T, typename... VariantTypes>
concept is_one_of = (std::is_same_v<T, VariantTypes> || ...);

template <typename T, typename Variant>
constexpr bool is_in_variant_v = false;

template <typename T, typename... Types>
constexpr bool is_in_variant_v<T, std::variant<Types...>> = is_one_of<T, Types...>;

} // namespace type_traits

} // namespace CS
