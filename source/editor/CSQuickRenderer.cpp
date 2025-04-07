#include "CSQuickRenderer.h"
#include <QOpenGLContext>
#include <QOpenGLExtraFunctions>

namespace CSEditor
{

CSQuickRenderer::CSQuickRenderer() {}

CSQuickRenderer::~CSQuickRenderer() {}

void CSQuickRenderer::render()
{
    auto gl = QOpenGLContext::currentContext()->extraFunctions();
    gl->glClearColor(0.5f, 0.6f, 0.7f, 1.0f);
    gl->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
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