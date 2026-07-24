#pragma once
#include <algorithm>
#include <string_view>

#define __MSVC__ 1

namespace CS
{

template <size_t N>
struct LiteralString final
{
    static constexpr size_t Size = N - 1;

    constexpr LiteralString() = default;
    constexpr LiteralString(const char (&str)[N]) noexcept { std::copy_n(str, N, value); }

    template <size_t M>
        requires(M <= N)
    constexpr LiteralString(const LiteralString<M>& other) noexcept
    {
        std::copy_n(other.value, M, value);
    }

    constexpr LiteralString(std::string_view other) noexcept { std::copy_n(other.data(), other.size(), value); }

    template <size_t M>
        requires(M <= N)
    constexpr LiteralString& operator=(const LiteralString<M>& other) noexcept
    {
        std::copy_n(other.value, M, value);
        return *this;
    }

    constexpr operator std::string_view() const noexcept { return std::string_view(value); }

    constexpr bool operator==(std::string_view other) const noexcept { return std::string_view(value) == other; }

    char value[N]{}; // include '\0'
};

template <size_t N1, size_t N2>
constexpr bool operator==(const LiteralString<N1>& lhs, const LiteralString<N2>& rhs) noexcept
{
    return std::string_view(lhs) == std::string_view(rhs);
}

} // namespace CS
