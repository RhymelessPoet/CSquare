#include "QuickRenderView.h"
#include "QuickRenderer.h"

namespace CSEditor
{

QuickRenderView::QuickRenderView() {}

QuickRenderView::~QuickRenderView() noexcept {}

QQuickRhiItemRenderer* QuickRenderView::createRenderer()
{
    return new QuickRenderer();
}

} // namespace CSEditor
