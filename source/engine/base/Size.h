#pragma once
#include <cassert>
#include <cstdint>
#include <initializer_list>
#include <limits>

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
    Size2(T width, T height) : m_width(static_cast<T>(width)), m_height(static_cast<T>(height))
    {}
    ~Size2() = default;

    const DataType& Width() const { return m_width; }
    const DataType& Height() const { return m_height; }

    float AspectRatioWH() const;
    float AspectRatioHW() const;

private:
    DataType m_width{0};
    DataType m_height{0};
};

using Size2u = Size2<uint32_t>;

template <typename DataType>
    requires std::is_arithmetic_v<DataType>
inline float Size2<DataType>::AspectRatioWH() const
{
    assert(std::numeric_limits<DataType>::epsilon() < m_height);
    return static_cast<float>(m_width) / m_height;
}

template <typename DataType>
    requires std::is_arithmetic_v<DataType>
inline float Size2<DataType>::AspectRatioHW() const
{
    assert(std::numeric_limits<DataType>::epsilon() < m_width);
    return static_cast<float>(m_height) / m_width;
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
        DataType data[3];
    };
};

using Size3U = Size3<uint32_t>;

} // namespace CS
