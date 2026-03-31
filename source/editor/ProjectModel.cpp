#include "ProjectModel.h"
#include "Engine.h"
#include "RenderModule.h"
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
        renderModule->GetMainView()->SetScene(impl().scene);
    }
    initializeScene();
    createSceneTreeModel();
}

const CS::UUID& ProjectModel::GetUUID() const
{
    return impl().uuid;
}

const TreeModel* ProjectModel::GetSceneTreeModel() const
{
    return &(impl().sceneTree);
}

TreeModel* ProjectModel::GetSceneTreeModel()
{
    return &(impl().sceneTree);
}

void ProjectModel::initializeScene()
{
    impl().sky = std::make_unique<PanoramicSky>(impl().scene);
    auto image = std::make_shared<Image>("assets/hdr/moonrise_puresky_4k.hdr", ImageFormat::RGB32Float);
    impl().sky->SetImage(image);

    // auto assetScene = AssetManager::Instance().GetAssetScene("assets/shape/cube.gltf");
    // auto assetScene = AssetManager::Instance().GetAssetScene("assets/model/monkeysun/monkeysun.gltf");
    // auto assetScene = AssetManager::Instance().GetAssetScene("assets/model/room/room.gltf");
    auto assetScene = AssetManager::Instance().GetAssetScene("C:/Users/Moke/Documents/Assets/sponza/sponza.gltf");

    if (assetScene != nullptr) {
        AssetImporter importer(impl().scene);
        auto sceneObject = importer.Import(assetScene->GetRoot());
    } else {
        // std::cerr << "Failed to load asset scene.\n";
    }
}

void ProjectModel::createSceneTreeModel()
{
    impl().sceneTree = CSEditor::MakeSceneTreeModel(impl().scene);
}

} // namespace CSEditor
