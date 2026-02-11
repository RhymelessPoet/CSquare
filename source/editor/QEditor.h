#pragma once
#include "QuickTreeModel.h"
#include <QObject>
#include <QString>
#include <memory>

namespace CSEditor
{
class ProjectModel;
class QuickTreeModel;

class QEditor : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QuickTreeModel* sceneTree READ getSceneTreeModel)
public:
    QEditor(/* args */);
    ~QEditor();

    Q_INVOKABLE void loadProject();
    Q_INVOKABLE const QString getProjectID() const;
    QuickTreeModel* getSceneTreeModel() { return m_sceneTreeModel.get(); }

private:
    void updateModels(ProjectModel* project);

private:
    QString m_projectID;
    std::unique_ptr<QuickTreeModel> m_sceneTreeModel;
};

} // namespace CSEditor
