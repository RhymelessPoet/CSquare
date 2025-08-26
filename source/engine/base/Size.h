#pragma once
#include <cassert>
#include <cstdint>
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

} // namespace CS
