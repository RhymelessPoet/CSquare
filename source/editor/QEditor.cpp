#include "QEditor.h"
#include "ProjectManager.h"
#include "ProjectModel.h"
#include "QuickSceneObjectModel.h"
#include "QuickTreeModel.h"
#include "model/SceneObjectModel.h"
#include "model/TreeNode.h"
#include "scene/SceneObject.h"

namespace CSEditor
{
QEditor::QEditor()
{
    m_sceneTreeModel = std::make_unique<QuickTreeModel>();
    m_sceneObjectDomain = std::make_unique<SceneObjectModel>();
    m_sceneObjectAdapter = std::make_unique<QuickSceneObjectModel>(m_sceneObjectDomain.get());
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

void QEditor::setSelectedSceneObject(const QModelIndex& index)
{
    if (m_sceneObjectDomain == nullptr) {
        return;
    }
    if (!index.isValid()) {
        m_sceneObjectDomain->Clear();
        return;
    }
    auto* node = reinterpret_cast<TreeNode*>(index.internalPointer());
    if (node == nullptr) {
        m_sceneObjectDomain->Clear();
        return;
    }
    const std::any underlying = node->GetUnderlyingNode();
    if (underlying.type() != typeid(std::shared_ptr<CS::SceneObject>)) {
        m_sceneObjectDomain->Clear();
        return;
    }
    const auto& so = std::any_cast<const std::shared_ptr<CS::SceneObject>&>(underlying);
    m_sceneObjectDomain->SetSceneObject(so);
}

void QEditor::clearSelectedSceneObject()
{
    if (m_sceneObjectDomain != nullptr) {
        m_sceneObjectDomain->Clear();
    }
}

void QEditor::updateModels(ProjectModel* project)
{
    m_sceneTreeModel->setModel(project->GetSceneTreeModel());
}

} // namespace CSEditor
