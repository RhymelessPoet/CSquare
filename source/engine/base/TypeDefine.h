#pragma once
#include <cstddef>
#include <cstdint>

namespace CS
{

enum class DataType : uint8_t
{
    Unknown = 0u,
    Byte,
    Int16,
    Int32,
    Int64,
    UInt16,
    UInt32,
    UInt64,
    Float32,
    Float64,
    Boolean,
    Max
};

namespace type_traits
{

template <typename T>
constexpr DataType data_type = DataType::Unknown;

template <>
constexpr DataType data_type<std::byte> = DataType::Byte;
template <>
constexpr DataType data_type<int16_t> = DataType::Int16;
template <>
constexpr DataType data_type<int32_t> = DataType::Int32;
template <>
constexpr DataType data_type<int64_t> = DataType::Int64;
template <>
constexpr DataType data_type<uint16_t> = DataType::UInt16;
template <>
constexpr DataType data_type<uint32_t> = DataType::UInt32;
template <>
constexpr DataType data_type<uint64_t> = DataType::UInt64;
template <>
constexpr DataType data_type<float> = DataType::Float32;
template <>
constexpr DataType data_type<double> = DataType::Float64;
template <>
constexpr DataType data_type<bool> = DataType::Boolean;

} // namespace type_traits

static inline size_t ByteSize(DataType type)
{
    if (type_traits::data_type<std::byte> == type) {
        return sizeof(std::byte);
    } else if (type_traits::data_type<int16_t> == type) {
        return sizeof(int16_t);
    } else if (type_traits::data_type<int32_t> == type) {
        return sizeof(int32_t);
    } else if (type_traits::data_type<int64_t> == type) {
        return sizeof(int64_t);
    } else if (type_traits::data_type<uint16_t> == type) {
        return sizeof(uint16_t);
    } else if (type_traits::data_type<uint32_t> == type) {
        return sizeof(uint32_t);
    } else if (type_traits::data_type<uint64_t> == type) {
        return sizeof(uint64_t);
    } else if (type_traits::data_type<float> == type) {
        return sizeof(float);
    } else if (type_traits::data_type<double> == type) {
        return sizeof(double);
    } else if (type_traits::data_type<bool> == type) {
        return sizeof(bool);
    }
    return 0u;
}

} // namespace CS
