#pragma once
#include "base/PImpl.h"
#include <functional>
#include <future>

namespace CS
{
class AssetScene;
class Scene;
class UUID;
} // namespace CS

namespace CSEditor
{
class TreeModel;
class ProjectModel : public CS::PImpl<ProjectModel>
{
public:
    ProjectModel(/* args */);

    const CS::UUID& GetUUID() const;
    const TreeModel* GetSceneTreeModel() const;
    TreeModel* GetSceneTreeModel();
    std::shared_ptr<CS::Scene> GetScene() const;

    // Synchronous scene setup (sky + directional light) and hierarchy tree
    // creation. Must be invoked AFTER the engine renderer/main View is
    // ready and BEFORE startAssetLoad().
    void initializeScene();

    // Kick off asynchronous GLTF/asset loading. initializeScene() must
    // have been called first.
    void startAssetLoad();

    bool pollAsyncLoad();
    void setOnAssetLoaded(std::function<void()> callback);

private:
    void createSceneTreeModel();
    void onAssetLoaded(std::shared_ptr<CS::AssetScene> assetScene);

private:
    std::future<std::shared_ptr<CS::AssetScene>> m_assetLoadFuture;
    std::function<void()> m_onAssetLoaded;
};

} // namespace CSEditor
