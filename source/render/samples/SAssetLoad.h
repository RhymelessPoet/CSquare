#pragma once

#include "IRenderSample.h"

namespace CS
{

class Scene;
class PanoramicSky;
class Light;

class SAssetLoad : public IRenderSample
{
public:
    SAssetLoad(std::shared_ptr<SceneObjectComposer> composer);
    ~SAssetLoad();

    CS_DELETE_COPY_MOVE(SAssetLoad)

    virtual void Initialize(std::shared_ptr<View> view) override;

    virtual void OnUpdate() override;

private:
    std::shared_ptr<Scene> m_scene;
    std::unique_ptr<PanoramicSky> m_sky;
    std::unique_ptr<Light> m_directionalLight;
};

} // namespace CS
