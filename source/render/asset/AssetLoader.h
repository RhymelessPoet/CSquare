#pragma once
#include "base/FileSystem.h"
#include "base/PImpl.h"
#include <future>

namespace CS
{

class Scene;
class SceneObject;
class AssetScene;

class AssetLoader : public PSharedImpl<AssetLoader>
{
public:
    AssetLoader(/* args */);

    std::shared_ptr<AssetScene> Load(const Path& path);
    std::future<std::shared_ptr<AssetScene>> LoadAsync(const Path& path);
};

} // namespace CS
