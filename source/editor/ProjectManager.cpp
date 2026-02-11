#include "ProjectManager.h"
#include "ProjectModel.h"
#include "base/utils/UUID.h"

namespace CSEditor
{
ProjectID ProjectManager::LoadProject(const CS::Path& path)
{
    return ProjectID();
}

ProjectID ProjectManager::CreateProject(const CS::Path& path)
{
    auto project = std::make_unique<ProjectModel>();
    auto projectID = project->GetUUID().ID();
    return m_projects.emplace(projectID, std::move(project)).first->first;
}

bool ProjectManager::SaveProject(ProjectID id)
{
    return false;
}

ProjectModel* ProjectManager::GetProject(ProjectID id)
{
    if (auto it = m_projects.find(id); it != m_projects.end()) {
        return it->second.get();
    }
    return nullptr;
}

} // namespace CSEditor
