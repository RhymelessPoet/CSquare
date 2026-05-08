#include "AssetLoader.h"
#include "base/Logger.h"
#include "impl/AssimpLoader.h"
#include <chrono>

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
    auto t0 = std::chrono::steady_clock::now();
    auto scene = impl().loader->Load(path);
    CS::LogPerf(::CS::BuiltInChannels::Asset(),
                CS::Fmt("AssetLoader::Load total: {:.2f}ms for '{}'",
                        std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - t0).count(),
                        path.filename().string()));
    return scene;
}

std::future<std::shared_ptr<AssetScene>> AssetLoader::LoadAsync(const Path& path)
{
    return std::async(std::launch::async, [this, path]() { return Load(path); });
}

} // namespace CS