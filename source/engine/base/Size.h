#pragma once
#include <cstdint>

namespace CS
{

template <typename DataType>
class Size2
{
public:
    Size2() = default;
    Size2(DataType width, DataType height) : m_width(width), m_height(height) {}
    ~Size2() = default;

    const DataType& Width() const { return m_width; }
    const DataType& Height() const { return m_height; }

private:
    DataType m_width{0};
    DataType m_height{0};
};

using Size2U = Size2<uint32_t>;

} // namespace CS
