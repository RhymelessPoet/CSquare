#pragma once
#include "graphics/RenderTarget.h"
#include <memory>

namespace CS
{
class ViewImpl;
class RenderContext;
class Scene;
class Camera;

class View final
{
public:
    View();
    View(RenderTarget target);
    ~View();

    void SetRenderTarget(RenderTarget target);
    RenderTarget GetRenderTarget();

    void SetScene(std::shared_ptr<Scene> scene);
    std::shared_ptr<Scene> GetScene() const;

    std::shared_ptr<Camera> GetCamera() const;

    void Render(RenderContext& context);

private:
    std::unique_ptr<ViewImpl> m_impl;
};

} // namespace CS
