#include "QuickRenderer.h"
#include "Engine.h"
#include "GLRendererBuilder.h"
#include "RenderModule.h"
#include "Texture.h"
#include "View.h"
#include "graphics/GraphicsAPI.h"
#include <QOpenGLContext>
#include <rhi/qrhi.h>

namespace CSEditor
{

QuickRenderer::QuickRenderer() {}

QuickRenderer::~QuickRenderer()
{
    if (m_texture != nullptr) {
        m_texture->destroy();
        delete m_texture;
    }
}

void QuickRenderer::initialize(QRhiCommandBuffer* cb)
{
    auto backend = rhi()->backend();
    if (rhi()->backend() == QRhi::OpenGLES2) {
        auto glContext = static_cast<const QRhiGles2NativeHandles*>(rhi()->nativeHandles())->context;
        HGLRC wglContext = glContext->nativeInterface<QNativeInterface::QWGLContext>()->nativeContext();

        CS::GLRendererBuilder rendererBuilder;
        rendererBuilder.SetSharedContext(wglContext);

        m_texture = rhi()->newTexture(QRhiTexture::RGBA8, QSize(1, 1));
        m_texture->create();

        auto& engine = CS::Engine::Instance();
        auto renderModule = CS::Engine::Instance().GetModule<CS::RenderModule>();
        if (renderModule.has_value() && m_view == nullptr) {
            renderModule.value()->CreateRenderer(rendererBuilder);
            m_view = renderModule.value()->CreateView();
            auto graphicsAPI = renderModule.value()->GetGraphicsAPI(m_view);
            auto texture = graphicsAPI->CreateTexture();
            texture.SetNativeTexture(static_cast<uint32_t>(m_texture->nativeTexture().object));
            auto renderTarget = graphicsAPI->CreateRenderTarget(CS::Size2U(1, 1));
            renderTarget.SetColorAttachment(texture);
            m_view->SetRenderTarget(renderTarget);
        }
    }
}

void QuickRenderer::render(QRhiCommandBuffer* cb)
{
    const QColor clearColor = QColor::fromRgbF(0.7f, 0.3f, 0.2f, 1.0f);
    cb->beginPass(renderTarget(), clearColor, {1.0f, 0});

    cb->endPass();
}

void QuickRenderer::synchronize(QQuickRhiItem* item)
{
    auto viewSize = item->size().toSize();
    if (viewSize != m_texture->pixelSize()) {
        m_texture->destroy();
        m_texture->setPixelSize(viewSize);
        m_texture->create();

        if (m_view != nullptr) {
            auto renderTaget = m_view->GetRenderTarget();
            renderTaget.SetSize(CS::Size2U(viewSize.width(), viewSize.height()));
            renderTaget.Build();
        }
    }
}

} // namespace CSEditor