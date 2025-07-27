#include "View.h"
#include "GraphicsCommandBuffer.h"
#include "RenderContext.h"
#include "RenderTarget.h"
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

void View::Render(RenderContext& context)
{
    context.GetCommandBuffer().BeginPass(GetRenderTarget()).Clear(Color(0.0f, 0.0f, 1.0f, 1.0f)).EndPass();
}

} // namespace CS
