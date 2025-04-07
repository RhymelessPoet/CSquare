#include "CSQuickRenderView.h"
#include "CSQuickRenderer.h"

namespace CSEditor
{

CSQuickRenderView::CSQuickRenderView() {}

CSQuickRenderView::~CSQuickRenderView() noexcept {}

QQuickFramebufferObject::Renderer* CSQuickRenderView::createRenderer() const
{
    return new CSQuickRenderer();
}

} // namespace CSEditor
