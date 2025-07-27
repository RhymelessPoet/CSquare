#pragma once
#include "RenderTarget.h"
#include <memory>

namespace CS
{
class ViewImpl;
class RenderContext;

class View final
{
public:
    View();
    View(RenderTarget target);
    ~View();

    void SetRenderTarget(RenderTarget target);
    RenderTarget GetRenderTarget();

    void Render(RenderContext& context);

private:
    std::unique_ptr<ViewImpl> m_impl;
};

} // namespace CS
