#include "SAssetLoad.h"

#include "asset/AssetImporter.h"
#include "asset/AssetLoader.h"
#include "asset/AssetManager.h"
#include "asset/AssetScene.h"
#include "asset/Image.h"

#include "scene/Light.h"
#include "scene/PanoramicSky.h"
#include "scene/Scene.h"
#include "scene/SceneObject.h"

#include "scene/View.h"
#include <iostream>

namespace CS
{
SAssetLoad::SAssetLoad(std::shared_ptr<SceneObjectComposer> composer) : IRenderSample(std::move(composer)) {}

SAssetLoad::~SAssetLoad() = default;

void SAssetLoad::Initialize(std::shared_ptr<View> view)
{
    m_scene = std::make_shared<CS::Scene>(m_composer);

    m_sky = std::make_unique<PanoramicSky>(m_scene);
    auto image = std::make_shared<Image>("assets/hdr/moonrise_puresky_4k.hdr", ImageFormat::RGB32Float);
    m_sky->SetImage(image);

    // A directional light is required before the scene's PBR material dispatches
    // a Shadow_Map requisite view: ViewGraph::configShadowView only creates the
    // shadow view's camera if the scene already has a Directional light, otherwise
    // the shadow view ends up with a null camera and Scene::OnRender crashes on
    // camera->GetPosition() during the first render.
    m_directionalLight = std::make_unique<Light>(m_scene);
    m_directionalLight->SetLightType(ELightType::Make<"Directional">());
    m_directionalLight->SetDirection(-Vector3f{0.5f, 1.0f, 0.3f});
    m_directionalLight->SetColor(Vector3f{1.0f, 1.0f, 1.0f});
    m_directionalLight->SetIntensity(5.1f);

    view->SetScene(m_scene);

     //auto assetScene = CS::AssetManager::Instance().GetAssetScene("assets/model/monkeysun/monkeysun.gltf");
    auto assetScene = CS::AssetManager::Instance().GetAssetScene("assets/model/monkeysun/monkeysun.gltf");
    //auto assetScene = CS::AssetManager::Instance().GetAssetScene("assets/shape/cube.gltf");

    if (assetScene != nullptr) {
        AssetImporter importer(m_scene);
        auto sceneObject = importer.Import(assetScene->GetRoot());
    } else {
        std::cerr << "Failed to load asset scene.\n";
    }
}

void SAssetLoad::OnUpdate() {}

} // namespace CS
