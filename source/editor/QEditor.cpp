#include "QEditor.h"
#include "ProjectManager.h"
#include "ProjectModel.h"

namespace CSEditor
{
QEditor::QEditor() = default;

QEditor::~QEditor() = default;

void QEditor::loadProject()
{
    m_projectID = ProjectManager::Instance().CreateProject().data();
}

const QString QEditor::getProjectID() const
{
    return m_projectID;
}

} // namespace CSEditor
