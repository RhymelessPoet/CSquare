#pragma once
#include "IRenderSample.h"

namespace CS
{
class Scene;

class HelloTriangles : public IRenderSample
{
public:
    HelloTriangles(/* args */) = default;

    virtual void Initialize(std::shared_ptr<View> view) override;

private:
    std::shared_ptr<Scene> m_scene;
};

} // namespace CS
