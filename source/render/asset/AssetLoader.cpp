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
    return impl().loader->Load(path);
}

} // namespace CS
