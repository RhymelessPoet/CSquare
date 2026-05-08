#include "ProjectModel.h"
#include "Engine.h"
#include "RenderModule.h"
#include "Scene/Light.h"
#include "SceneTreeModel.h"
#include "asset/AssetImporter.h"
#include "asset/AssetLoader.h"
#include "asset/AssetManager.h"
#include "asset/AssetScene.h"
#include "asset/Image.h"
#include "base/utils/UUID.h"
#include "scene/PanoramicSky.h"
#include "scene/View.h"
#include <unordered_map>

using namespace CS;

template <>
struct ImplData<CSEditor::ProjectModel>
{
    UUID uuid;

    std::shared_ptr<Scene> scene;
    std::unique_ptr<PanoramicSky> sky;
    std::unique_ptr<Light> directionalLight;

    CSEditor::TreeModel sceneTree;
};
namespace CSEditor
{

ProjectModel::ProjectModel() : ImplBase()
{
    impl().uuid.Regenerate();
    auto _renderModule = Engine::Instance().GetModule<RenderModule>();
    if (_renderModule.has_value()) {
        auto renderModule = _renderModule.value();
        impl().scene = std::make_shared<Scene>(renderModule->GetSOComposer());
        // The main View may not yet exist if the engine renderer is still
        // being initialized asynchronously (see QuickRenderView). In that
        // case the scene will be attached when QuickRenderView finishes
        // initializing by calling ProjectManager::AttachAllToView().
        if (auto mainView = renderModule->GetMainView()) {
            mainView->SetScene(impl().scene);
        }
    }
    // NOTE: Scene setup (sky/light) and scene-hierarchy tree creation are
    // deferred to initializeScene(), which is invoked by
    // QEditor::beginAssetLoading() AFTER the engine renderer/main view is
    // ready and BEFORE any asset loading begins.
}

const CS::UUID& ProjectModel::GetUUID() const
{
    return impl().uuid;
}

std::shared_ptr<CS::Scene> ProjectModel::GetScene() const
{
    return impl().scene;
}

const TreeModel* ProjectModel::GetSceneTreeModel() const
{
    return &(impl().sceneTree);
}

TreeModel* ProjectModel::GetSceneTreeModel()
{
    return &(impl().sceneTree);
}

void ProjectModel::setOnAssetLoaded(std::function<void()> callback)
{
    m_onAssetLoaded = std::move(callback);
}

void ProjectModel::initializeScene()
{
    // Synchronous scene setup: sky + directional light. No asset I/O.
    impl().sky = std::make_unique<PanoramicSky>(impl().scene);
    auto image = std::make_shared<Image>("assets/hdr/moonrise_puresky_4k.hdr", ImageFormat::RGB32Float);
    impl().sky->SetImage(image);

    impl().directionalLight = std::make_unique<Light>(impl().scene);
    impl().directionalLight->SetLightType(ELightType::Make<"Directional">());
    impl().directionalLight->SetDirection(-Vector3f{0.5f, 1.0f, 0.3f});
    impl().directionalLight->SetColor(Vector3f{1.0f, 1.0f, 1.0f});
    impl().directionalLight->SetIntensity(5.1f);

    // Create the scene-hierarchy tree now so QML's SceneHierarchyView
    // reflects the sky + directional light BEFORE asset loading begins.
    createSceneTreeModel();
}

void ProjectModel::startAssetLoad()
{
    // Path assetPath = "assets/shape/cube.gltf";
    Path assetPath = "assets/model/monkeysun/monkeysun.gltf";
    // Path assetPath = "assets/model/room/room.gltf";
    // Path assetPath = "C:/Users/Moke/Documents/Assets/sponza/sponza.gltf";
    // Path assetPath = "C:/Users/Moke/Documents/Assets/road_bike/road_bike.gltf";
    // Path assetPath = "C:/Users/Moke/Documents/Assets/lost_empire/lost_empire.gltf";
    // Path assetPath = "C:/Users/Moke/Documents/Assets/san_miguel/san_miguel.gltf";
    // Path assetPath = "C:/Users/Moke/Documents/Assets/fireplace_room/fireplace_room.gltf";

    m_assetLoadFuture = AssetManager::Instance().GetAssetSceneAsync(assetPath);
}

bool ProjectModel::pollAsyncLoad()
{
    if (!m_assetLoadFuture.valid()) {
        return true;
    }
    if (m_assetLoadFuture.wait_for(std::chrono::seconds(0)) != std::future_status::ready) {
        return false;
    }
    auto assetScene = m_assetLoadFuture.get();
    onAssetLoaded(assetScene);
    return true;
}

void ProjectModel::onAssetLoaded(std::shared_ptr<CS::AssetScene> assetScene)
{
    if (assetScene != nullptr) {
        AssetImporter importer(impl().scene);
        auto sceneObject = importer.Import(assetScene->GetRoot());
    }
    createSceneTreeModel();
    if (m_onAssetLoaded) {
        m_onAssetLoaded();
    }
}

void ProjectModel::createSceneTreeModel()
{
    impl().sceneTree = CSEditor::MakeSceneTreeModel(impl().scene);
}

} // namespace CSEditor
