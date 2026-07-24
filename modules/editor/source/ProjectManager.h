#pragma once
#include "FileSystem.h"
#include "Singleton.h"
#include "utils/UUID.h"
#include <memory>
#include <unordered_map>

namespace CS
{
class View;
} // namespace CS

namespace CSEditor
{

using ProjectID = std::string_view;

class ProjectModel;

class ProjectManager : public CS::Singleton<ProjectManager>
{
public:
    ProjectID LoadProject(const CS::Path& path);
    ProjectID CreateProject(const CS::Path& path = {});
    bool SaveProject(ProjectID id);

    ProjectModel* GetProject(ProjectID id);

    // Attach every known project's scene to the supplied view. Used by
    // QuickRenderView once its engine renderer is ready, in case projects
    // were created before the view existed.
    void AttachAllToView(const std::shared_ptr<CS::View>& view);

private:
    friend class CS::Singleton<ProjectManager>;
    ProjectManager() = default;

private:
    std::unordered_map<ProjectID, std::unique_ptr<ProjectModel>> m_projects;
};

} // namespace CSEditor
