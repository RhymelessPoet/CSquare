#include "CSQuickRenderView.h"
#include "CSQuickRenderer.h"

namespace CSEditor
{

CSQuickRenderView::CSQuickRenderView() {}

CSQuickRenderView::~CSQuickRenderView() noexcept {}

QQuickRhiItemRenderer* CSQuickRenderView::createRenderer()
{
    return new CSQuickRenderer();
}

} // namespace CSEditor
