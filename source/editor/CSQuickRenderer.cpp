#include "CSQuickRenderer.h"
#include "RenderModule.h"
#include <QOpenGLContext>
#include <QOpenGLExtraFunctions>

namespace CSEditor
{

CSQuickRenderer::CSQuickRenderer() {}

CSQuickRenderer::~CSQuickRenderer() {}

void CSQuickRenderer::render()
{
    CS::RenderModule(CS::EGraphicAPI::OpenGL).Update();
}

QOpenGLFramebufferObject* CSQuickRenderer::createFramebufferObject(const QSize& size)
{
    return QQuickFramebufferObject::Renderer::createFramebufferObject(size);
}

void CSQuickRenderer::synchronize(QQuickFramebufferObject* fbo)
{
    QQuickFramebufferObject::Renderer::synchronize(fbo);
}

} // namespace CSEditor