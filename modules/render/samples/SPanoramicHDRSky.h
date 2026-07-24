#pragma once
#include "IRenderSample.h"

namespace CS
{
class Scene;
class PanoramicSky;

class SPanoramicHDRSky : public IRenderSample
{
public:
    SPanoramicHDRSky(std::shared_ptr<SceneObjectComposer> composer);
    ~SPanoramicHDRSky();

    virtual void Initialize(std::shared_ptr<View> view) override;

    virtual void OnUpdate() override;

private:
    std::shared_ptr<Scene> m_scene;
    std::unique_ptr<PanoramicSky> m_sky;
};

} // namespace CS
