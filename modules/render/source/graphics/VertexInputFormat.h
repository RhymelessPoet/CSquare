#pragma once
#include "TypeDefine.h"
#include <cstdint>

namespace CS
{

enum class VertexInputFormat
{
    Float = 0,
    Float2,
    Float3,
    Float4,
    Int,
    Int2,
    Int3,
    Int4,
    UInt,
    UInt2,
    UInt3,
    UInt4,
    Max
};

struct VertexFormatSize
{
    VertexFormatSize() = default;
    VertexFormatSize(size_t size, uint32_t count) : componentSize(size), componentCount(count) {}

    uint32_t componentSize{32u};
    uint32_t componentCount{1u};
};

constexpr inline VertexFormatSize VertexInputFormatSize(VertexInputFormat format)
{
    switch (format) {
    case VertexInputFormat::Float:
        return {sizeof(float), 1u};
    case VertexInputFormat::Float2:
        return {sizeof(float), 2u};
    case VertexInputFormat::Float3:
        return {sizeof(float), 3u};
    case VertexInputFormat::Float4:
        return {sizeof(float), 4u};
    case VertexInputFormat::Int:
        return {sizeof(int), 1u};
    case VertexInputFormat::Int2:
        return {sizeof(int), 2u};
    case VertexInputFormat::Int3:
        return {sizeof(int), 3u};
    case VertexInputFormat::Int4:
        return {sizeof(int), 4u};
    case VertexInputFormat::UInt:
        return {sizeof(unsigned int), 1u};
    case VertexInputFormat::UInt2:
        return {sizeof(unsigned int), 2u};
    case VertexInputFormat::UInt3:
        return {sizeof(unsigned int), 3u};
    case VertexInputFormat::UInt4:
        return {sizeof(unsigned int), 4u};
    default:
        return {}; // Invalid format
    }
}

constexpr inline VertexInputFormat VertexInputFormatFrom(DataType type, uint32_t componentCount)
{
    switch (type) {
    case DataType::Float32:
        switch (componentCount) {
        case 1:
            return VertexInputFormat::Float;
        case 2:
            return VertexInputFormat::Float2;
        case 3:
            return VertexInputFormat::Float3;
        case 4:
            return VertexInputFormat::Float4;
        default:
            return VertexInputFormat::Max;
        }
    case DataType::Int32:
        switch (componentCount) {
        case 1:
            return VertexInputFormat::Int;
        case 2:
            return VertexInputFormat::Int2;
        case 3:
            return VertexInputFormat::Int3;
        case 4:
            return VertexInputFormat::Int4;
        default:
            return VertexInputFormat::Max;
        }
    case DataType::UInt32:
        switch (componentCount) {
        case 1:
            return VertexInputFormat::UInt;
        case 2:
            return VertexInputFormat::UInt2;
        case 3:
            return VertexInputFormat::UInt3;
        case 4:
            return VertexInputFormat::UInt4;
        default:
            return VertexInputFormat::Max;
        }
    default:
        return VertexInputFormat::Max;
    }
}

} // namespace CS
