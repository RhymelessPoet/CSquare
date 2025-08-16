#pragma once
#include "RenderTarget.h"
#include <memory>

namespace CS
{
class ViewImpl;
class RenderContext;
class Scene;

class View final
{
public:
    View();
    View(RenderTarget target);
    ~View();

    void SetRenderTarget(RenderTarget target);
    RenderTarget GetRenderTarget();

    void SetScene(std::shared_ptr<Scene> scene);
    std::shared_ptr<Scene> GetScene() const { return m_scene; }

    void Render(RenderContext& context);

private:
    std::unique_ptr<ViewImpl> m_impl;
    std::shared_ptr<Scene> m_scene;
};

} // namespace CS
