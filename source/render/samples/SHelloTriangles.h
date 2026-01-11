#pragma once
#include "IRenderSample.h"

namespace CS
{
class Scene;
class SceneObject;

class SHelloTriangles : public IRenderSample
{
public:
    SHelloTriangles(std::shared_ptr<SceneObjectComposer> composer);

    virtual void Initialize(std::shared_ptr<View> view) override;

    virtual void OnUpdate() override;

private:
    std::shared_ptr<Scene> m_scene;
    std::shared_ptr<SceneObject> m_groupRoot;
};

} // namespace CS
