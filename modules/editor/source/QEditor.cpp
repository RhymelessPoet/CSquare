#include "QEditor.h"
#include "ProjectManager.h"
#include "ProjectModel.h"
#include "QuickSceneObjectModel.h"
#include "QuickTreeModel.h"
#include "Logger.h"
#include "model/SceneObjectModel.h"
#include "model/TreeNode.h"
#include "scene/SceneObject.h"
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QProcess>


namespace CSEditor
{
QEditor::QEditor()
{
    m_sceneTreeModel = std::make_unique<QuickTreeModel>();
    m_sceneObjectDomain = std::make_unique<SceneObjectModel>();
    m_sceneObjectAdapter = std::make_unique<QuickSceneObjectModel>(m_sceneObjectDomain.get());
    m_sceneSelection = std::make_unique<QItemSelectionModel>(m_sceneTreeModel.get());

    // Route selection changes from the shared QItemSelectionModel directly
    // into the domain model. Both Hierarchy and Inspector bind to this
    // single selection source.
    QObject::connect(m_sceneSelection.get(), &QItemSelectionModel::currentChanged, this,
                     &QEditor::onSelectionCurrentChanged);
}

QEditor::~QEditor() = default;

void QEditor::loadProject()
{
    auto projecIDView = ProjectManager::Instance().CreateProject();
    m_projectID = QString::fromStdString(std::string(projecIDView));
    auto project = ProjectManager::Instance().GetProject(m_projectID.toStdString());
    updateModels(project);
    // Do NOT start asset loading here. The project's scene tree is ready
    // but the engine renderer/main view is created asynchronously by
    // QuickRenderView. beginAssetLoading() is invoked from QML when
    // CSQuickRenderView emits engineReady.
}

void QEditor::beginAssetLoading()
{
    auto project = ProjectManager::Instance().GetProject(m_projectID.toStdString());
    if (project == nullptr) {
        return;
    }
    // 1) Populate the scene with sky + directional light and build the
    //    scene-hierarchy tree BEFORE any asset I/O. This guarantees the
    //    QML SceneHierarchyView reflects the base scene immediately.
    project->initializeScene();
    updateModels(project);

    // 2) Kick off the async GLTF load and begin polling.
    project->startAssetLoad();
    startAsyncLoadPolling(project);
}

void QEditor::startAsyncLoadPolling(ProjectModel* project)
{
    if (project == nullptr) {
        return;
    }
    project->setOnAssetLoaded([this, project]() {
        updateModels(project);
        emit sceneAssetLoaded();
        if (m_asyncLoadTimer != nullptr) {
            m_asyncLoadTimer->stop();
        }
    });

    m_asyncLoadTimer = std::make_unique<QTimer>(this);
    QObject::connect(m_asyncLoadTimer.get(), &QTimer::timeout, [this, project]() {
        if (project->pollAsyncLoad()) {
            m_asyncLoadTimer->stop();
        }
    });
    m_asyncLoadTimer->start(100);
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

QStringList QEditor::getSampleNames() const
{
    // Samples are deployed by the editor build into <editor-dir>/samples.
    // We scan the directory once per call so the menu reflects whatever
    // is currently on disk.
    const QString samplesDir = QCoreApplication::applicationDirPath() + QStringLiteral("/samples");
    QDir dir(samplesDir);
    if (!dir.exists()) {
        return {};
    }
    const QStringList filters{QStringLiteral("*.exe")};
    const QFileInfoList entries = dir.entryInfoList(filters, QDir::Files | QDir::NoSymLinks, QDir::Name);
    QStringList names;
    names.reserve(entries.size());
    for (const QFileInfo& fi : entries) {
        names.push_back(fi.completeBaseName());
    }
    return names;
}

bool QEditor::launchSample(const QString& sampleName) const
{
    if (sampleName.isEmpty()) {
        return false;
    }
    const QString appDir = QCoreApplication::applicationDirPath();
    const QString exePath = appDir + QStringLiteral("/samples/") + sampleName + QStringLiteral(".exe");
    if (!QFileInfo::exists(exePath)) {
        CS::LogError(CS::BuiltInChannels::Editor(),
                     CS::Fmt("QEditor: sample executable not found: {}", exePath.toStdString()));
        return false;
    }

    // Working directory is the sample's own folder so Windows picks up
    // any side-by-side runtime DLLs first. The samples folder also
    // receives a copy of the shared asset tree via the build so relative
    // "assets/..." paths continue to resolve.
    const QString workingDir = appDir + QStringLiteral("/samples");
    qint64 pid = 0;
    const bool ok = QProcess::startDetached(exePath, QStringList{}, workingDir, &pid);
    if (!ok) {
        CS::LogError(CS::BuiltInChannels::Editor(),
                     CS::Fmt("QEditor: failed to launch sample: {}", exePath.toStdString()));
        return false;
    }
    CS::LogInfo(CS::BuiltInChannels::Editor(),
                CS::Fmt("QEditor: launched sample '{}' (pid={})", sampleName.toStdString(), pid));
    return true;
}

void QEditor::updateModels(ProjectModel* project)
{
    m_sceneTreeModel->setModel(project->GetSceneTreeModel());
}

void QEditor::onSelectionCurrentChanged(const QModelIndex& current, const QModelIndex& /*previous*/)
{
    if (current.isValid()) {
        setSelectedSceneObject(current);
    } else {
        clearSelectedSceneObject();
    }
}

} // namespace CSEditor
