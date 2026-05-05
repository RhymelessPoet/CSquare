#pragma once
#include "QuickSceneObjectModel.h"
#include "QuickTreeModel.h"
#include <QModelIndex>
#include <QObject>
#include <QString>
#include <memory>

namespace CSEditor
{
class ProjectModel;
class QuickTreeModel;
class SceneObjectModel;

class QEditor : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QuickTreeModel* sceneTree READ getSceneTreeModel)
    Q_PROPERTY(QuickSceneObjectModel* sceneObjectModel READ getSceneObjectModel CONSTANT)
public:
    QEditor(/* args */);
    ~QEditor();

    Q_INVOKABLE void loadProject();
    Q_INVOKABLE const QString getProjectID() const;
    Q_INVOKABLE void setSelectedSceneObject(const QModelIndex& index);
    Q_INVOKABLE void clearSelectedSceneObject();
    QuickTreeModel* getSceneTreeModel() { return m_sceneTreeModel.get(); }
    QuickSceneObjectModel* getSceneObjectModel() { return m_sceneObjectAdapter.get(); }

private:
    void updateModels(ProjectModel* project);

private:
    QString m_projectID;
    std::unique_ptr<QuickTreeModel> m_sceneTreeModel;
    std::unique_ptr<SceneObjectModel> m_sceneObjectDomain;
    std::unique_ptr<QuickSceneObjectModel> m_sceneObjectAdapter;
};

} // namespace CSEditor
