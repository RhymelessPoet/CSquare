#include "Mesh.h"

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

} // namespace CS
