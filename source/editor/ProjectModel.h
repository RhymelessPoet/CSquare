#pragma once
#include "base/PImpl.h"

namespace CS
{
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

private:
    void initializeScene();
    void createSceneTreeModel();
};

} // namespace CSEditor
