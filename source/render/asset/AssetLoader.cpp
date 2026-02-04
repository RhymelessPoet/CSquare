#include "AssetLoader.h"
#include "impl/AssimpLoader.h"

namespace CS
{

template <>
struct ImplData<AssetLoader>
{
    std::unique_ptr<IAssetLoaderImpl> loader;
};

AssetLoader::AssetLoader()
{
    impl().loader = std::make_unique<AssimpLoader>();
}

std::shared_ptr<AssetScene> AssetLoader::Load(const Path& path)
{
    if (!std::filesystem::exists(path)) {
        // TODO: log error
        return nullptr;
    }
    return impl().loader->Load(path);
}

} // namespace CS
