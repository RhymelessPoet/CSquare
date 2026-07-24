#include "AssetNode.h"

namespace CS
{
AssetNode::AssetNode(std::shared_ptr<AssetNode> parent) : m_parent(parent) {}

void AssetNode::AddMesh(std::shared_ptr<Mesh> mesh, std::shared_ptr<MaterialInstance> matereial)
{
    m_meshs.emplace_back(std::move(mesh), std::move(matereial));
}

} // namespace CS
