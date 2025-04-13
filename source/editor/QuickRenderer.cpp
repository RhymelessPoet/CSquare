#include "QuickRenderer.h"
#include "RenderModule.h"

#include <rhi/qrhi.h>

namespace CSEditor
{

QuickRenderer::QuickRenderer() {}

QuickRenderer::~QuickRenderer() {}

void QuickRenderer::initialize(QRhiCommandBuffer* cb) {}

void QuickRenderer::render(QRhiCommandBuffer* cb)
{
    const QColor clearColor = QColor::fromRgbF(0.7f, 0.3f, 0.2f, 1.0f);
    cb->beginPass(renderTarget(), clearColor, {1.0f, 0});

    cb->endPass();
}

void QuickRenderer::synchronize(QQuickRhiItem* item) {}

} // namespace CSEditor