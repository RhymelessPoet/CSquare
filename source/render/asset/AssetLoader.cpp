#include "AssetLoader.h"
#include "base/Logger.h"
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
        CS::LogError(::CS::BuiltInChannels::Asset(), CS::Fmt("Asset not found: {}", path.string()));
        return nullptr;
    }
    return impl().loader->Load(path);
}

} // namespace CS