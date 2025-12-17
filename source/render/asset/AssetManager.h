#pragma once
#include "base/Singleton.h"
#include <filesystem>
#include <memory>
#include <string>
#include <unordered_map>

namespace CS
{

class AssetScene;
class AssetNode;
class Material;
class Mesh;
class AssetLoader;

class AssetManager : public Singleton<AssetManager>
{
public:
    friend class Singleton<AssetManager>;
    std::shared_ptr<AssetScene> GetAssetScene(const std::filesystem::path& path);
    std::shared_ptr<Material> GetMaterial(const std::string& name);
    std::shared_ptr<Mesh> GetMesh(const std::string& name);

private:
    AssetManager();

private:
    std::shared_ptr<AssetLoader> m_loader;
    std::unordered_map<std::filesystem::path, std::shared_ptr<AssetScene>> m_assetRoots;
    std::unordered_map<std::string, std::shared_ptr<Material>> m_materials;
};

} // namespace CS
