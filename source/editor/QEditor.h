#pragma once
#include "QuickSceneObjectModel.h"
#include "QuickTreeModel.h"
#include <QItemSelectionModel>
#include <QModelIndex>
#include <QObject>
#include <QString>
#include <QTimer>
#include <memory>

namespace CSEditor
{
class ProjectModel;
class QuickTreeModel;
class SceneObjectModel;

// Dataflow (MVVM):
//   QML (Hierarchy) -> QItemSelectionModel -> QEditor::setSelectedSceneObject
//     -> SceneObjectModel::SetSceneObject -> listener
//     -> QuickSceneObjectModel::onDomainChanged -> QML (Inspector)
//
//   QML edit -> QuickComponentModel::setValue -> PropertyItem::SetValue
//     -> UDRefl write -> dataChanged -> QML widgets re-read.
class QEditor : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QuickTreeModel* sceneTree READ getSceneTreeModel)
    Q_PROPERTY(QuickSceneObjectModel* sceneObjectModel READ getSceneObjectModel CONSTANT)
    Q_PROPERTY(QItemSelectionModel* sceneSelection READ getSceneSelection CONSTANT)
public:
    QEditor(/* args */);
    ~QEditor();

    Q_INVOKABLE void loadProject();
    Q_INVOKABLE const QString getProjectID() const;
    Q_INVOKABLE void setSelectedSceneObject(const QModelIndex& index);
    Q_INVOKABLE void clearSelectedSceneObject();
    QuickTreeModel* getSceneTreeModel() { return m_sceneTreeModel.get(); }
    QuickSceneObjectModel* getSceneObjectModel() { return m_sceneObjectAdapter.get(); }
    QItemSelectionModel* getSceneSelection() { return m_sceneSelection.get(); }

private:
    void updateModels(ProjectModel* project);
    void onSelectionCurrentChanged(const QModelIndex& current, const QModelIndex& previous);

private:
    void startAsyncLoadPolling(ProjectModel* project);

private:
    QString m_projectID;
    std::unique_ptr<QuickTreeModel> m_sceneTreeModel;
    std::unique_ptr<SceneObjectModel> m_sceneObjectDomain;
    std::unique_ptr<QuickSceneObjectModel> m_sceneObjectAdapter;
    std::unique_ptr<QItemSelectionModel> m_sceneSelection;
    std::unique_ptr<QTimer> m_asyncLoadTimer;
};

} // namespace CSEditor
