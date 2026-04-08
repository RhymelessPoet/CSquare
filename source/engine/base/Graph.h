#pragma once
#include "GenerationID.h"
#include "TypeTraits.h"
#include <expected>
#include <unordered_map>
#include <unordered_set>

namespace CS
{

template <GenerationIDType VertexID>
class DirectedEdges
{
public:
    DirectedEdges() = default;

    bool AddEdge(const VertexID& from, const VertexID& to) { return m_edges[from].insert(to).second; }

    bool RemoveEdge(const VertexID& from, const VertexID& to)
    {
        if (auto it = m_edges.find(from); it != m_edges.end()) {
            return it->second.erase(to) > 0;
        }
        return false;
    }

    bool RemoveEdges(const VertexID& from) { return m_edges.erase(from) > 0; }

    const std::unordered_set<VertexID>& GetEdgesFrom(const VertexID& from) const
    {
        static const std::unordered_set<VertexID> emptySet;

        if (auto it = m_edges.find(from); it != m_edges.end()) {
            return it->second;
        }

        return emptySet;
    }

    std::vector<VertexID> GetTopologicallySortedVertices(std::unordered_map<VertexID, uint32_t>& initInDegreeMap) const;

private:
    std::unordered_map<VertexID, std::unordered_set<VertexID>> m_edges;
};

template <typename VertexData, GenerationIDType VertexID>
class DirectedAcyclicGraph final
{
public:
    struct Vertex
    {
        VertexID id;
        VertexData data;
    };

    DirectedAcyclicGraph(/* args */) = default;

    VertexID AddVertex(const VertexData& data)
    {
        VertexID id = m_vertexIDGenerator.Allocate();
        m_vertices.emplace(id, Vertex{id, data});
        return id;
    }

    VertexID AddVertex()
    {
        VertexID id = m_vertexIDGenerator.Allocate();
        m_vertices.emplace(id, Vertex{id, VertexData{}});
        return id;
    }

    bool SetVertexData(const VertexID& id, const VertexData& data)
    {
        if (auto it = m_vertices.find(id); it != m_vertices.end()) {
            it->second.data = data;
            return true;
        }
        return false;
    }

    bool RemoveVertex(const VertexID& id)
    {
        m_edges.RemoveEdges(id);
        m_vertexIDGenerator.Free(id);
        return m_vertices.erase(id) > 0;
    }

    bool AddEdge(const VertexID& from, const VertexID& to)
    {
        if (m_vertices.find(from) == m_vertices.end() || m_vertices.find(to) == m_vertices.end()) {
            return false;
        }
        return m_edges.AddEdge(from, to);
    }

    bool RemoveEdge(const VertexID& from, const VertexID& to) { return m_edges.RemoveEdge(from, to); }

    std::expected<const Vertex*, bool> GetVertex(const VertexID& id) const
    {
        if (auto it = m_vertices.find(id); it != m_vertices.end()) {
            return &it->second;
        }
        return std::unexpected(false);
    }

    std::vector<VertexID> GetTopologicallySortedVertices() const;

private:
    VertexID::Generator m_vertexIDGenerator;
    std::unordered_map<VertexID, Vertex> m_vertices;
    DirectedEdges<VertexID> m_edges;
};

template <typename VertexData, GenerationIDType VertexID>
using DAG = DirectedAcyclicGraph<VertexData, VertexID>;

template <GenerationIDType VertexID>
inline std::vector<VertexID>
DirectedEdges<VertexID>::GetTopologicallySortedVertices(std::unordered_map<VertexID, uint32_t>& initInDegreeMap) const
{
    auto& inDegreeMap = initInDegreeMap;
    for (const auto& [from, toSet] : m_edges) {
        for (const auto& to : toSet) {
            inDegreeMap[to]++;
        }
    }
    std::vector<VertexID> sortedVertices;
    std::vector<VertexID> zeroInDegreeVertices;
    for (const auto& [id, inDegree] : inDegreeMap) {
        if (inDegree == 0) {
            zeroInDegreeVertices.push_back(id);
        }
    }
    while (!zeroInDegreeVertices.empty()) {
        auto current = zeroInDegreeVertices.back();
        zeroInDegreeVertices.pop_back();
        sortedVertices.push_back(current);
        for (const auto& to : GetEdgesFrom(current)) {
            inDegreeMap[to]--;
            if (inDegreeMap[to] == 0) {
                zeroInDegreeVertices.push_back(to);
            }
        }
    }
    return sortedVertices;
}

template <typename VertexData, GenerationIDType VertexID>
inline std::vector<VertexID> DirectedAcyclicGraph<VertexData, VertexID>::GetTopologicallySortedVertices() const
{
    std::unordered_map<VertexID, uint32_t> inDegreeMap;
    for (const auto& [id, vertex] : m_vertices) {
        inDegreeMap[id] = 0;
    }
    return m_edges.GetTopologicallySortedVertices(inDegreeMap);
}

} // namespace CS
