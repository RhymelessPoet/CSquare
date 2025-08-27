#pragma once
#include <span>
#include <vector>

namespace CS
{

class Mesh
{
public:
    Mesh(/* args */);
    Mesh(std::vector<float> vertices, std::vector<uint32_t> indices);
    ~Mesh();

    std::span<const float> GetVertexData() const;
    std::span<const uint32_t> GetIndexData() const;

private:
    std::vector<float> m_vertices;   // Vertex data
    std::vector<uint32_t> m_indices; // Index data
};

} // namespace CS
