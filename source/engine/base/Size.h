#pragma once
#include <cassert>
#include <cstdint>
#include <initializer_list>
#include <limits>
#include <tuple>

namespace CS
{

template <typename DataType>
    requires std::is_arithmetic_v<DataType>
class Size2
{
public:
    Size2() = default;
    template <typename T>
        requires std::is_arithmetic_v<T>
    Size2(T x, T y) : width(static_cast<T>(x)), height(static_cast<T>(y))
    {}
    ~Size2() = default;

    float AspectRatioWH() const;
    float AspectRatioHW() const;

    union {
        struct
        {
            DataType x;
            DataType y;
        };
        struct
        {
            DataType u;
            DataType v;
        };
        struct
        {
            DataType width;
            DataType height;
        };
        struct
        {
            DataType width;
            DataType height;
        };
        DataType data[2] = {DataType(0), DataType(0)};
    };
};

using Size2u = Size2<uint32_t>;

template <typename DataType>
    requires std::is_arithmetic_v<DataType>
inline float Size2<DataType>::AspectRatioWH() const
{
    assert(std::numeric_limits<DataType>::epsilon() < height);
    return static_cast<float>(width) / height;
}

template <typename DataType>
    requires std::is_arithmetic_v<DataType>
inline float Size2<DataType>::AspectRatioHW() const
{
    assert(std::numeric_limits<DataType>::epsilon() < width);
    return static_cast<float>(height) / width;
}

template <typename DataType>
    requires std::is_arithmetic_v<DataType>
class Size3
{
public:
    template <typename P>
        requires std::is_arithmetic_v<P>
    Size3(std::initializer_list<P> list)
    {
        int i = 0;
        for (auto it = list.begin(); it != list.end() && i < 3; ++it, ++i) {
            data[i] = static_cast<P>(*it);
        }
    }

    Size3() : x(0), y(0), z(0) {}

    Size2<DataType> XY() const { return Size2<DataType>(x, y); }
    Size2<DataType> XZ() const { return Size2<DataType>(x, z); }
    Size2<DataType> YZ() const { return Size2<DataType>(y, z); }

    template <typename T>
        requires std::is_arithmetic_v<T>
    friend Size3<DataType> operator*(const Size3<DataType>& size, T scalar)
    {
        return Size3<DataType>{size.x * scalar, size.y * scalar, size.z * scalar};
    }

    template <typename T>
        requires std::is_arithmetic_v<T>
    friend Size3<DataType> operator*(T scalar, const Size3<DataType>& size)
    {
        return size * scalar;
    }

    union {
        struct
        {
            DataType x;
            DataType y;
            DataType z;
        };
        struct
        {
            DataType u;
            DataType v;
            DataType w;
        };
        struct
        {
            DataType width;
            DataType height;
            DataType depth;
        };
        struct
        {
            DataType width;
            DataType height;
            DataType channels;
        };
        DataType data[3] = {DataType(0), DataType(0), DataType(0)};
    };
};

using Size3u = Size3<uint32_t>;
using Size3d = Size3<double>;

// 重载 get 函数
template <std::size_t Index, typename T>
decltype(auto) get(const CS::Size2<T>& size)
{
    return size.data[Index];
}

template <std::size_t Index, typename T>
decltype(auto) get(const CS::Size3<T>& size)
{
    return size.data[Index];
}

} // namespace CS

namespace std
{

template <typename T>
struct tuple_size<CS::Size2<T>> : integral_constant<size_t, 2>
{
};

template <typename T>
struct tuple_size<CS::Size3<T>> : integral_constant<size_t, 3>
{
};

// 特化 std::tuple_element
template <size_t Index, typename T>
struct std::tuple_element<Index, CS::Size2<T>>
{
    using type = T;
};

template <size_t Index, typename T>
struct std::tuple_element<Index, CS::Size3<T>>
{
    using type = T;
};

} // namespace std