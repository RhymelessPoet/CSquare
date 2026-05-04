#pragma once

#include <algorithm>
#include <concepts>
#include <format>
#include <stdexcept>
#include <utility>

#include "base/Size.h"
#include "base/math/Vector.h"
#pragma once

#include <algorithm>
#include <concepts>
#include <format>
#include <stdexcept>
#include <utility>

#include "base/Size.h"
#include "base/math/Vector.h"

namespace CS
{

template <typename T>
concept Arithmetic = std::integral<T> || std::floating_point<T>;

template <Arithmetic T>
class Rect final
{
public:
    constexpr Rect() noexcept = default;
    constexpr Rect(T x, T y, T w, T h) noexcept : m_position{x, y}, m_size{w, h} {}
    constexpr Rect(const Vector3<T>& position, const Size2<T>& size) noexcept : m_position{position}, m_size{size} {}

    constexpr Rect(const Rect&) noexcept = default;
    constexpr Rect(Rect&&) noexcept = default;
    constexpr Rect& operator=(const Rect&) noexcept = default;
    constexpr Rect& operator=(Rect&&) noexcept = default;

    const Size2<T>& GetSize() const { return m_size; }
    const Vector2<T>& GetPosition() const { return m_position; }

    [[nodiscard]] constexpr T Right() const noexcept { return m_position.x + m_size.width; }
    [[nodiscard]] constexpr T Bottom() const noexcept { return m_position.y + m_size.height; }
    [[nodiscard]] constexpr T CenterX() const noexcept { return m_position.x + m_size.width / 2; }
    [[nodiscard]] constexpr T CenterY() const noexcept { return m_position.y + m_size.height / 2; }
    [[nodiscard]] constexpr T Area() const noexcept { return m_size.width * m_size.height; }
    [[nodiscard]] constexpr T Perimeter() const noexcept { return 2 * (m_size.width + m_size.height); }

    // 合法性
    [[nodiscard]] constexpr bool IsValid() const noexcept { return m_size.width > T{} && m_size.height > T{}; }

    // 位置/尺寸
    constexpr void MoveTo(T newX, T newY) noexcept
    {
        m_position.x = newX;
        m_position.y = newY;
    }

    constexpr void Offset(T dx, T dy) noexcept
    {
        m_position.x += dx;
        m_position.y += dy;
    }

    constexpr void Resize(T w, T h) noexcept
    {
        m_size.width = w;
        m_size.height = h;
    }

    [[nodiscard]] constexpr bool Contains(T px, T py) const noexcept
    {
        return px >= m_position.x && px < Right() && py >= m_position.y && py < Bottom();
    }

    [[nodiscard]] constexpr bool Contains(const Rect& other) const noexcept
    {
        return other.m_position.x >= m_position.x && other.Right() <= Right() && other.m_position.y >= m_position.y &&
               other.Bottom() <= Bottom();
    }

    [[nodiscard]] constexpr bool Intersects(const Rect& other) const noexcept
    {
        return m_position.x < other.Right() && Right() > other.m_position.x && m_position.y < other.Bottom() &&
               Bottom() > other.m_position.y;
    }

    [[nodiscard]] constexpr bool operator==(const Rect&) const noexcept = default;

    constexpr Rect& operator*=(T scale) noexcept
    {
        m_size.width *= scale;
        m_size.height *= scale;
        return *this;
    }

    [[nodiscard]] constexpr Rect operator*(T scale) const noexcept
    {
        Rect res = *this;
        res *= scale;
        return res;
    }

private:
    Vector2<T> m_position{};
    Size2<T> m_size{};
};

using Rect32i = Rect<int32_t>;
using Rect32u = Rect<uint32_t>;
using Rect32f = Rect<float>;
using Rect64f = Rect<double>;

template <Arithmetic T>
[[nodiscard]] constexpr Rect<T> Intersect(const Rect<T>& a, const Rect<T>& b) noexcept
{
    const T ix = std::max(a.m_position.x, b.m_position.x);
    const T iy = std::max(a.m_position.y, b.m_position.y);
    const T ir = std::min(a.Right(), b.Right());
    const T ib = std::min(a.Bottom(), b.Bottom());

    if (ir <= ix || ib <= iy)
        return {};
    return {ix, iy, ir - ix, ib - iy};
}

template <Arithmetic T>
[[nodiscard]] constexpr Rect<T> Bounds(const Rect<T>& a, const Rect<T>& b) noexcept
{
    const T nx = std::min(a.m_position.x, b.m_position.x);
    const T ny = std::min(a.m_position.y, b.m_position.y);
    const T nr = std::max(a.Right(), b.Right());
    const T nb = std::max(a.Bottom(), b.Bottom());
    return {nx, ny, nr - nx, nb - ny};
}

} // namespace CS
