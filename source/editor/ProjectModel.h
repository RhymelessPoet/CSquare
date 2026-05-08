#pragma once
#include "base/PImpl.h"
#include <functional>
#include <future>

namespace CS
{
class AssetScene;
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

    void initializeSceneAsync();
    bool pollAsyncLoad();
    void setOnAssetLoaded(std::function<void()> callback);

private:
    void initializeScene();
    void createSceneTreeModel();
    void onAssetLoaded(std::shared_ptr<CS::AssetScene> assetScene);

private:
    std::future<std::shared_ptr<CS::AssetScene>> m_assetLoadFuture;
    std::function<void()> m_onAssetLoaded;
};

} // namespace CSEditor
