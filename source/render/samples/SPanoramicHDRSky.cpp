#include "SPanoramicHDRSky.h"

#include "asset/Image.h"
#include "base/math/Math.h"
#include "graphics/VertexInputLayout.h"
#include "scene/CameraSystem.h"
#include "scene/MeshRenderSystem.h"
#include "scene/PanoramicSky.h"
#include "scene/Scene.h"
#include "scene/SceneObject.h"
#include "scene/TransformSystem.h"
#include "scene/View.h"

namespace CS
{
SPanoramicHDRSky::SPanoramicHDRSky(std::shared_ptr<SceneObjectComposer> composer) : IRenderSample(std::move(composer))
{}

SPanoramicHDRSky::~SPanoramicHDRSky() = default;

void SPanoramicHDRSky::Initialize(std::shared_ptr<View> view)
{
    m_scene = std::make_shared<CS::Scene>(m_composer);

    m_sky = std::make_unique<PanoramicSky>(m_scene);

    view->SetScene(m_scene);

    // auto image = std::make_shared<Image>("assets/hdr/clarens_night_01_4k.hdr", ImageFormat::RGB32Float);

    auto image = std::make_shared<Image>("assets/hdr/moonrise_puresky_4k.hdr", ImageFormat::RGB32Float);

    // auto image = std::make_shared<Image>("assets/hdr/starry_sky_aurora.hdr", ImageFormat::RGB32Float);
    m_sky->SetImage(image);
}

void SPanoramicHDRSky::OnUpdate() {}

} // namespace CS
