#pragma once
#include "base/TypeDefine.h"
#include "base/memory/Buffer.h"
#include "utils/AxisAlignedBoundingBox.h"
#include <functional>
#include <memory>
#include <string>

namespace CS
{

class Mesh
{
public:
    struct Attribute
    {
        std::string name;
        //                   |offset.........end|               or     |offset..............end|
        // [aaa...........aaa bbb............bbb ccc.......ccc]    [aaabbbccc.............aaabbbccc]
        uint32_t offset{0u};
        uint32_t end{0u};
        uint8_t bufferID{0u};
        uint8_t componentCount{0u};
        DataType type{DataType::Unknown};
    };
    class Builder
    {
    public:
        Builder();
        Builder& SetName(std::string_view name);
        Builder& SetVertexCount(uint32_t count);
        Builder& SetIndices(Buffer data, DataType type);
        Builder& SetAABB(const AABB& box);

        Builder& AddVertexBuffer(Buffer data);
        Builder& AddAttribute(const Attribute& attribute);

        std::shared_ptr<Mesh> Build();

    private:
        std::shared_ptr<Mesh> m_mesh;
    };

    std::string_view GetName() const { return m_name; }
    const AABB& GetAABB() const { return m_box; }

    size_t GetVertexBufferCount() const { return m_vertexBuffers.size(); }
    std::span<const std::byte> GetVertexBufferView(uint8_t bufferID) const;
    const Buffer& GetVertexBuffer(uint8_t bufferID) const;

    template <typename T>
    std::span<const T> GetIndices() const
    {
        constexpr auto indexType = type_traits::data_type<T>;
        if (m_indexType != indexType && DataType::Byte != indexType) {
            return std::span<const T>();
        }
        return m_indices.GetData<T>();
    }

    uint32_t GetVertexCount() const { return m_vertexCount; }

    void ForEachAttribute(const std::function<void(const Attribute&)>& func) const;

    void RecomputeAABB();

private:
    std::string m_name;
    AABB m_box;
    std::vector<Attribute> m_attributes;
    std::vector<Buffer> m_vertexBuffers;
    uint32_t m_vertexCount{0u};
    Buffer m_indices;
    DataType m_indexType{DataType::Unknown};
};

} // namespace CS
