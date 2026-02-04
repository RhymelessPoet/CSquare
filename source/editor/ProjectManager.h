#pragma once
#include "base/FileSystem.h"
#include "base/Singleton.h"
#include "base/utils/UUID.h"
#include <memory>
#include <unordered_map>

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

private:
    std::unordered_map<std::string_view, std::unique_ptr<ProjectModel>> m_projects;
};

} // namespace CSEditor
