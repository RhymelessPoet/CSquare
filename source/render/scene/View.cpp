#include "View.h"
#include "Camera.h"
#include "Scene.h"
#include "graphics/GraphicsCommandBuffer.h"
#include "graphics/RenderTarget.h"
#include "renderer/RenderContext.h"
#include <optional>

namespace CS
{
class ViewImpl
{
public:
    ViewImpl() = default;
    ~ViewImpl() = default;

public:
    std::optional<RenderTarget> m_renderTarget;
    std::shared_ptr<Scene> m_scene;
    std::shared_ptr<Camera> m_camera;
};

View::View()
{
    m_impl = std::make_unique<ViewImpl>();
}

View::View(RenderTarget target) : View()
{
    m_impl->m_renderTarget = target;
}

View::~View() {}

void View::SetRenderTarget(RenderTarget target)
{
    m_impl->m_renderTarget = target;
}

RenderTarget View::GetRenderTarget()
{
    return m_impl->m_renderTarget.value();
}

void View::SetScene(std::shared_ptr<Scene> scene)
{
    m_impl->m_scene = std::move(scene);
    m_impl->m_camera = m_impl->m_scene->CreateCamera();
}

std::shared_ptr<Scene> View::GetScene() const
{
    return m_impl->m_scene;
}

std::shared_ptr<Camera> View::GetCamera() const
{
    return m_impl->m_camera;
}

void View::Render(RenderContext& context)
{
    context.SetCamera(m_impl->m_camera);
    auto rtSize = GetRenderTarget().GetSize();
    auto cmdBuf = context.GetCommandBuffer();
    cmdBuf.BeginPass(GetRenderTarget())
        .Clear(Color(61.0f / 255.0f, 61.0f / 255.0f, 61.0f / 255.0f, 1.0f))
        .SetViewport(0, 0, rtSize.Width(), rtSize.Height());
    m_impl->m_scene->OnRender(context);
    cmdBuf.EndPass();
}

} // namespace CS
