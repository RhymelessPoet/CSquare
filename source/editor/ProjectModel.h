#pragma once
#include "base/PImpl.h"

namespace CS
{
class UUID;
} // namespace CS

namespace CSEditor
{

class ProjectModel : public CS::PImpl<ProjectModel>
{
public:
    ProjectModel(/* args */);

    const CS::UUID& GetUUID() const;

private:
    void initializeScene();
    void createSceneTreeModel();
};

} // namespace CSEditor
