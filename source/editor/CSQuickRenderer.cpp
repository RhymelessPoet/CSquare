#include "CSQuickRenderer.h"
#include "RenderModule.h"

#include <rhi/qrhi.h>

namespace CSEditor
{

CSQuickRenderer::CSQuickRenderer() {}

CSQuickRenderer::~CSQuickRenderer() {}

void CSQuickRenderer::initialize(QRhiCommandBuffer* cb) {}

void CSQuickRenderer::render(QRhiCommandBuffer* cb)
{
    const QColor clearColor = QColor::fromRgbF(0.7f, 0.3f, 0.2f, 1.0f);
    cb->beginPass(renderTarget(), clearColor, {1.0f, 0});

    cb->endPass();
}

void CSQuickRenderer::synchronize(QQuickRhiItem* item) {}

} // namespace CSEditor