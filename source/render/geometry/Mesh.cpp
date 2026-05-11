#include "Mesh.h"
#include "base/Logger.h"

namespace CS
{
Mesh::Builder::Builder() : m_mesh(std::make_shared<Mesh>()) {}

Mesh::Builder& Mesh::Builder::SetName(std::string_view name)
{
    m_mesh->m_name = name;
    return *this;
}

Mesh::Builder& Mesh::Builder::SetVertexCount(uint32_t count)
{
    m_mesh->m_vertexCount = count;
    return *this;
}

Mesh::Builder& Mesh::Builder::SetIndices(Buffer data, DataType type)
{
    m_mesh->m_indices = std::move(data);
    m_mesh->m_indexType = type;
    return *this;
}

Mesh::Builder& Mesh::Builder::SetAABB(const AABB& box)
{
    m_mesh->m_box = box;
    return *this;
}

Mesh::Builder& Mesh::Builder::AddVertexBuffer(Buffer data)
{
    m_mesh->m_vertexBuffers.emplace_back(std::move(data));
    return *this;
}

Mesh::Builder& Mesh::Builder::AddAttribute(const Attribute& attribute)
{
    m_mesh->m_attributes.push_back(attribute);
    return *this;
}

std::shared_ptr<Mesh> Mesh::Builder::Build()
{
    if (!m_mesh->m_box.IsValid()) {
        m_mesh->RecomputeAABB();
    }
    return m_mesh;
}

std::span<const std::byte> Mesh::GetVertexBufferView(uint8_t bufferID) const
{
    if (bufferID >= m_vertexBuffers.size()) {
        return std::span<const std::byte>();
    }
    return m_vertexBuffers[bufferID].GetData<std::byte>();
}

const Buffer& Mesh::GetVertexBuffer(uint8_t bufferID) const
{
    if (bufferID >= m_vertexBuffers.size()) {
        static Buffer invalidBuffer;
        return invalidBuffer;
    }
    return m_vertexBuffers[bufferID];
}

void Mesh::ForEachAttribute(const std::function<void(const Attribute&)>& func) const
{
    for (const auto& attr : m_attributes) {
        func(attr);
    }
}

void Mesh::RecomputeAABB()
{
    AABB box;
    ForEachAttribute([&](const Attribute& attr) {
        if (attr.name == "_position") {
            const Buffer& vb = GetVertexBuffer(attr.bufferID);
            auto bytes = vb.GetByteData();
            if (bytes.empty())
                return;

            const uint32_t span = attr.end - attr.offset;
            const uint32_t stride = span / m_vertexCount;
            const uint32_t minElemBytes = attr.componentCount * sizeof(float);
            if (stride < minElemBytes)
                return;

            for (uint32_t i = 0; i < m_vertexCount; ++i) {
                const std::byte* base = bytes.data() + attr.offset + i * stride;
                const float* p = reinterpret_cast<const float*>(base);
                Vector3d position{0.0, 0.0, 0.0};
                for (uint32_t j = 0; j < attr.componentCount && j < 3; ++j) {
                    position[j] = static_cast<double>(p[j]);
                }
                box.Include(position);
            }
        }
    });
    m_box = box;
}

} // namespace CS
