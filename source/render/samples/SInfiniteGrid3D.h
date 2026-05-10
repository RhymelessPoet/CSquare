#pragma once
#include "IRenderSample.h"

namespace CS
{
class Scene;
class View;
class InfiniteGrid3D;

class SInfiniteGrid3D : public IRenderSample
{
public:
    SInfiniteGrid3D(std::shared_ptr<SceneObjectComposer> composer);
    ~SInfiniteGrid3D();

    virtual void Initialize(std::shared_ptr<View> view) override;

    virtual void OnUpdate() override;

private:
    std::shared_ptr<Scene> m_scene;
    std::shared_ptr<View> m_view;
    std::unique_ptr<InfiniteGrid3D> m_grid;
};

} // namespace CS
