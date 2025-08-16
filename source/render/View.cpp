#include "View.h"
#include "GraphicsCommandBuffer.h"
#include "RenderContext.h"
#include "RenderTarget.h"
#include "Scene.h"
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
};

View::View()
{
    m_impl = std::make_unique<ViewImpl>();
}

View::View(RenderTarget target) {}

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
    m_scene = std::move(scene);
}

void View::Render(RenderContext& context)
{
    auto rtSize = GetRenderTarget().GetSize();
    auto cmdBuf = context.GetCommandBuffer();
    cmdBuf.BeginPass(GetRenderTarget())
        .Clear(Color(61.0f / 255.0f, 61.0f / 255.0f, 61.0f / 255.0f, 1.0f))
        .SetViewport(0, 0, rtSize.Width(), rtSize.Height());
    m_scene->OnRender(context);
    cmdBuf.EndPass();
}

} // namespace CS
