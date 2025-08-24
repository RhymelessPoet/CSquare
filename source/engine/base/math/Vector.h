#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <initializer_list>
#include <limits>
#include <type_traits>

namespace CS
{

template <typename T, uint32_t N>
    requires(N > 0 && N < 5) && (std::is_arithmetic_v<T>)
class Vector
{
public:
    Vector() = default;
    Vector(const Vector& other) { m_data = other.m_data; }
    Vector(std::initializer_list<T> list)
    {
        int i = 0;
        for (auto it = list.begin(); it != list.end() && i < N; ++it, ++i) {
            m_data[i] = *it;
        }
        for (; i < N; ++i) {
            m_data[i] = T();
        }
    }

    T& operator[](int i) { return m_data[i]; }

    const T& operator[](int i) const { return m_data[i]; }

    const T& X() const { return m_data[0]; }
    const T& Y() const { return m_data[1]; }
    const T& Z() const
        requires(N >= 3)
    {
        return m_data[2];
    }
    const T& W() const
        requires(N >= 4)
    {
        return m_data[3];
    }

    Vector operator+(const Vector& rhs) const
    {
        Vector result;
        for (int i = 0; i < N; ++i)
            result.m_data[i] = m_data[i] + rhs.m_data[i];
        return result;
    }

    constexpr Vector operator-(const Vector& rhs) const
    {
        Vector result;
        for (int i = 0; i < N; ++i)
            result.m_data[i] = m_data[i] - rhs.m_data[i];
        return result;
    }

    constexpr Vector operator-() const
    {
        Vector result;
        for (int i = 0; i < N; ++i)
            result.m_data[i] = -m_data[i];
        return result;
    }

    constexpr Vector operator*(T scalar) const
    {
        Vector result;
        for (int i = 0; i < N; ++i)
            result.m_data[i] = m_data[i] * scalar;
        return result;
    }

    constexpr Vector operator/(T scalar) const
    {
        Vector result;
        for (int i = 0; i < N; ++i)
            result.m_data[i] = m_data[i] / scalar;
        return result;
    }

    constexpr Vector& operator+=(const Vector& rhs)
    {
        for (int i = 0; i < N; ++i)
            m_data[i] += rhs.m_data[i];
        return *this;
    }

    constexpr Vector& operator-=(const Vector& rhs)
    {
        for (int i = 0; i < N; ++i)
            m_data[i] -= rhs.m_data[i];
        return *this;
    }

    constexpr Vector& operator*=(T scalar)
    {
        for (int i = 0; i < N; ++i)
            m_data[i] *= scalar;
        return *this;
    }

    constexpr Vector& operator/=(T scalar)
    {
        for (int i = 0; i < N; ++i)
            m_data[i] /= scalar;
        return *this;
    }

    bool operator==(const Vector& rhs) const
    {
        for (int i = 0; i < N; ++i)
            if (m_data[i] != rhs.m_data[i])
                return false;
        return true;
    }

    bool operator!=(const Vector& rhs) const { return !(*this == rhs); }

    void SetZero() { std::fill(m_data, m_data + N, T()); }

    void Set(const T (&arr)[N]) { std::copy(arr, arr + N, m_data); }

    constexpr T Dot(const Vector& rhs) const
    {
        T result = T();
        for (int i = 0; i < N; ++i)
            result += m_data[i] * rhs.m_data[i];
        return result;
    }

    Vector Cross(const Vector& rhs) const
        requires(N == 3)
    {
        return Vector3<T>({m_data[1] * rhs.m_data[2] - m_data[2] * rhs.m_data[1],
                           m_data[2] * rhs.m_data[0] - m_data[0] * rhs.m_data[2],
                           m_data[0] * rhs.m_data[1] - m_data[1] * rhs.m_data[0]});
    }

    T Length() const { return std::sqrt(Dot(*this)); }

    T LengthSquared() const { return Dot(*this); }

    constexpr Vector Normalized() const
    {
        T len = Length();
        return (*this) / len;
    }

    constexpr void Normalize()
    {
        auto len = Length();
        if (len != std::numeric_limits<T>::zero()) {
            for (int i = 0; i < N; ++i) {
                m_data[i] /= len;
            }
        }
    }

    constexpr bool Equal(const Vector& rhs)
        requires std::is_integral_v<T>
    {
        for (int i = 0; i < N; ++i)
            if (m_data[i] != rhs.m_data[i])
                return false;
        return true;
    }

    constexpr bool Equal(const Vector& rhs, T epsion = std::numeric_limits<T>::epsion())
        requires(std::is_floating_point_v<T>)
    {
        for (int i = 0; i < N; ++i) {
            if (std::abs(m_data[i] - rhs.m_data[i]) > epsion)
                return false;
        }
        return true;
    }

    constexpr void Clamp(const Vector& minVector, const Vector& maxVector)
    {
        for (int i = 0; i < N; ++i)
            m_data[i] = std::max(minVector.m_data[i], std::min(m_data[i], maxVector.m_data[i]));
    }

    void Fill(const T& value) { std::fill(m_data, m_data + N, value); }

    template <std::size_t I>
    const auto& get() const
        requires(I < N)
    {
        return m_data[I];
    }

    template <std::size_t I>
    auto& get()
        requires(I < N)
    {
        return m_data[I];
    }

private:
    std::array<T, N> m_data;
};

// Type aliases for 2D, 3D, 4D vectors
template <typename T>
using Vector2 = Vector<T, 2>;
template <typename T>
using Vector3 = Vector<T, 3>;
template <typename T>
using Vector4 = Vector<T, 4>;

using Vector2f = Vector2<float>;
using Vector2d = Vector2<double>;
using Vector3f = Vector3<float>;
using Vector3d = Vector3<double>;
using Vector4f = Vector4<float>;
using Vector4d = Vector4<double>;

} // namespace CS

namespace std
{
template <typename T, uint32_t N>
struct tuple_size<CS::Vector<T, N>> : integral_constant<size_t, N>
{
};

template <std::size_t I, typename T, uint32_t N>
struct tuple_element<I, CS::Vector<T, N>>
{
    using type = T;
};
} // namespace std
