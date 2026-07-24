#pragma once
#include <memory>
#include <type_traits>
#include <variant>
#include <vector>

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

template <typename T>
concept enum_type = requires { std::is_enum_v<T>; };

template <typename T>
constexpr bool is_shared_ptr_v = false;
template <typename T>
constexpr bool is_weak_ptr_v = false;
template <typename T>
constexpr bool is_unique_ptr_v = false;

template <typename U>
constexpr bool is_shared_ptr_v<std::shared_ptr<U>> = true;
template <typename U>
constexpr bool is_weak_ptr_v<std::weak_ptr<U>> = true;
template <typename U>
constexpr bool is_unique_ptr_v<std::unique_ptr<U>> = true;

template <typename T>
concept smart_pointer = is_shared_ptr_v<T> || is_weak_ptr_v<T> || is_unique_ptr_v<T>;

template <typename T>
concept weak_pointer = is_weak_ptr_v<T>;

template <typename T>
constexpr bool is_vector_v = false;

template <typename T>
constexpr bool is_vector_v<std::vector<T>> = true;

template <typename T, template <typename...> typename TargetTemplate>
struct is_template_instance : std::false_type
{
};

template <template <typename...> typename TargetTemplate, typename... Args>
struct is_template_instance<TargetTemplate<Args...>, TargetTemplate> : std::true_type
{
};

template <typename T, template <typename...> typename TargetTemplate>
constexpr bool is_template_instance_v = is_template_instance<T, TargetTemplate>::value;

template <typename F, typename Ret, typename... Args>
concept callable = std::invocable<F, Args...> && std::is_same_v<std::invoke_result_t<F, Args...>, Ret>;

} // namespace type_traits

} // namespace CS
