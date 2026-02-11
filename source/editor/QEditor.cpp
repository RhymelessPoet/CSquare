#include "QEditor.h"
#include "ProjectManager.h"
#include "ProjectModel.h"
#include "QuickTreeModel.h"

namespace CSEditor
{
QEditor::QEditor()
{
    m_sceneTreeModel = std::make_unique<QuickTreeModel>();
}

QEditor::~QEditor() = default;

void QEditor::loadProject()
{
    auto projecIDView = ProjectManager::Instance().CreateProject();
    m_projectID = QString::fromStdString(std::string(projecIDView));
    auto project = ProjectManager::Instance().GetProject(m_projectID.toStdString());
    updateModels(project);
}

const QString QEditor::getProjectID() const
{
    return m_projectID;
}

void QEditor::updateModels(ProjectModel* project)
{
    m_sceneTreeModel->setModel(project->GetSceneTreeModel());
}

} // namespace CSEditor
