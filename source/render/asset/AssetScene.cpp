#include "AssetScene.h"
#include "AssetNode.h"

namespace CS
{
AssetScene::AssetScene(std::string_view name) : m_name(name), m_root(std::make_shared<AssetNode>()) {}

void AssetScene::AddTexture(std::shared_ptr<Image> texture)
{
    m_textures.push_back(std::move(texture));
}

std::shared_ptr<Image> AssetScene::GetTexture(uint32_t index) const
{
    if (index < m_textures.size()) {
        return m_textures[index];
    }
    return std::shared_ptr<Image>();
}

void AssetScene::AddMaterial(std::shared_ptr<MaterialInstance> material)
{
    m_materials.push_back(std::move(material));
}

std::shared_ptr<MaterialInstance> AssetScene::GetMaterial(uint32_t index) const
{
    if (index < m_materials.size()) {
        return m_materials[index];
    }
    return std::shared_ptr<MaterialInstance>();
}

void AssetScene::AddMesh(std::shared_ptr<Mesh> mesh, uint32_t materialIndex)
{
    m_meshs.push_back(std::move(mesh));
    m_meshMaterialIndices.push_back(materialIndex);
}

std::shared_ptr<Mesh> AssetScene::GetMesh(uint32_t index) const
{
    if (index < m_meshs.size()) {
        return m_meshs[index];
    }
    return std::shared_ptr<Mesh>();
}

std::optional<uint32_t> AssetScene::GetMaterialIndex(uint32_t meshIndex) const
{
    if (meshIndex < m_meshMaterialIndices.size()) {
        return m_meshMaterialIndices[meshIndex];
    }
    return std::nullopt;
}

} // namespace CS
