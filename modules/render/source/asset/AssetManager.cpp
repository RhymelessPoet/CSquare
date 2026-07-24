#include "AssetManager.h"
#include "AssetLoader.h"

namespace CS
{
AssetManager::AssetManager()
{
    m_loader = std::make_shared<AssetLoader>();
}

std::shared_ptr<AssetScene> AssetManager::GetAssetScene(const std::filesystem::path& path)
{
    auto itr = m_assetRoots.find(path);
    if (itr != m_assetRoots.end()) {
        return itr->second;
    }

    auto assetScene = m_loader->Load(path);
    m_assetRoots[path] = assetScene;

    return assetScene;
}

std::future<std::shared_ptr<AssetScene>> AssetManager::GetAssetSceneAsync(const std::filesystem::path& path)
{
    return m_loader->LoadAsync(path);
}

std::shared_ptr<Material> AssetManager::GetMaterial(const std::string& name)
{
    return std::shared_ptr<Material>();
}

std::shared_ptr<Mesh> AssetManager::GetMesh(const std::string& name)
{
    return std::shared_ptr<Mesh>();
}
} // namespace CS
