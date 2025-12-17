#pragma once
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace CS
{

class Image;
class MaterialInstance;
class Mesh;
class AssetNode;

class AssetScene
{
public:
    explicit AssetScene(std::string_view name = "");

    std::shared_ptr<AssetNode> GetRoot() const { return m_root; }

    void AddTexture(std::shared_ptr<Image> texture);
    std::shared_ptr<Image> GetTexture(uint32_t index) const;

    void AddMaterial(std::shared_ptr<MaterialInstance> material);
    std::shared_ptr<MaterialInstance> GetMaterial(uint32_t index) const;

    void AddMesh(std::shared_ptr<Mesh> mesh, uint32_t materialIndex);
    std::shared_ptr<Mesh> GetMesh(uint32_t index) const;
    std::optional<uint32_t> GetMaterialIndex(uint32_t meshIndex) const;

private:
    std::string m_name;
    std::shared_ptr<AssetNode> m_root;

    std::vector<std::shared_ptr<Image>> m_textures;
    std::vector<std::shared_ptr<MaterialInstance>> m_materials;
    std::vector<std::shared_ptr<Mesh>> m_meshs;
    std::vector<uint32_t> m_meshMaterialIndices;
};

} // namespace CS
