#include "SAssetLoad.h"

#include "asset/AssetImporter.h"
#include "asset/AssetLoader.h"
#include "asset/AssetManager.h"
#include "asset/AssetScene.h"
#include "asset/Image.h"

#include "scene/PanoramicSky.h"
#include "scene/Scene.h"
#include "scene/SceneObject.h"

#include "scene/View.h"

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

    view->SetScene(m_scene);

    auto assetScene = CS::AssetManager::Instance().GetAssetScene("assets/model/monkeysun/monkeysun.gltf");

    AssetImporter importer(m_scene);
    auto sceneObject = importer.Import(assetScene->GetRoot());
}

void SAssetLoad::OnUpdate() {}

} // namespace CS
