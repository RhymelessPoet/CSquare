#include "ProjectModel.h"
#include "Engine.h"
#include "RenderModule.h"
#include "asset/AssetImporter.h"
#include "asset/AssetLoader.h"
#include "asset/AssetManager.h"
#include "asset/AssetScene.h"
#include "asset/Image.h"
#include "base/utils/UUID.h"
#include "scene/PanoramicSky.h"
#include "scene/Scene.h"
#include "scene/View.h"

using namespace CS;
template <>
struct ImplData<CSEditor::ProjectModel>
{
    UUID uuid;

    std::shared_ptr<Scene> scene;
    std::unique_ptr<PanoramicSky> sky;
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
}

const CS::UUID& ProjectModel::GetUUID() const
{
    return impl().uuid;
}

void ProjectModel::initializeScene()
{
    impl().sky = std::make_unique<PanoramicSky>(impl().scene);
    auto image = std::make_shared<Image>("assets/hdr/moonrise_puresky_4k.hdr", ImageFormat::RGB32Float);
    impl().sky->SetImage(image);

    auto assetScene = AssetManager::Instance().GetAssetScene("assets/shape/cube.gltf");

    if (assetScene != nullptr) {
        AssetImporter importer(impl().scene);
        auto sceneObject = importer.Import(assetScene->GetRoot());
    } else {
        // std::cerr << "Failed to load asset scene.\n";
    }
}

void ProjectModel::createSceneTreeModel() {}

} // namespace CSEditor
