#include "Mesh.h"

namespace CS
{
Mesh::Mesh() {}

Mesh::Mesh(std::vector<float> vertices, std::vector<uint32_t> indices)
    : m_vertices(std::move(vertices)), m_indices(std::move(indices))
{}

Mesh::~Mesh() {}

std::span<const float> Mesh::GetVertexData() const
{
    return std::span<const float>(m_vertices);
}

std::span<const uint32_t> Mesh::GetIndexData() const
{
    return std::span<const uint32_t>(m_indices);
}

} // namespace CS
