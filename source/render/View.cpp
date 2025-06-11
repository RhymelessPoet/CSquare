#include "View.h"
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

} // namespace CS
