#include "QuickRenderer.h"
#include "Engine.h"
#include "GLRendererBuilder.h"
#include "QBlitRenderPass.h"
#include "RenderModule.h"
#include "Texture.h"
#include "View.h"
#include "graphics/GraphicsAPI.h"
#include <QOpenGLContext>
#include <rhi/qrhi.h>

namespace CSEditor
{

QuickRenderer::QuickRenderer() {}

QuickRenderer::~QuickRenderer() {}

void QuickRenderer::initialize(QRhiCommandBuffer* cb)
{
    auto backend = rhi()->backend();
    if (rhi()->backend() == QRhi::OpenGLES2) {
        // auto glContext = static_cast<const QRhiGles2NativeHandles*>(rhi()->nativeHandles())->context;
        // HGLRC wglContext = glContext->nativeInterface<QNativeInterface::QWGLContext>()->nativeContext();

        auto renderModule = CS::Engine::Instance().GetModule<CS::RenderModule>();
        if (renderModule.has_value() && m_view == nullptr) {
            CS::GLRendererBuilder rendererBuilder;
            // rendererBuilder.SetSharedContext(wglContext);

            QImage img(":/CSQML/qml/icons/cslogo.png");
            auto texture = rhi()->newTexture(QRhiTexture::RGBA8, img.size());
            m_texture = std::unique_ptr<QRhiTexture>(texture);
            m_texture->create();

            auto sampler =
                rhi()->newSampler(QRhiSampler::Filter::Linear, QRhiSampler::Filter::Linear, QRhiSampler::Filter::None,
                                  QRhiSampler::AddressMode::Repeat, QRhiSampler::AddressMode::Repeat);
            m_sampler = std::unique_ptr<QRhiSampler>(sampler);
            m_sampler->create();

            QRhiResourceUpdateBatch* batch = rhi()->nextResourceUpdateBatch();
            batch->uploadTexture(m_texture.get(), img.convertToFormat(QImage::Format_RGBA8888));

            renderModule.value()->CreateRenderer(rendererBuilder);
            m_view = renderModule.value()->CreateView();
            auto graphicsAPI = renderModule.value()->GetGraphicsAPI(m_view);
            graphicsAPI->Initialize();
            auto csTexture = graphicsAPI->CreateTexture();
            csTexture.SetSize(CS::Size2U(img.width(), img.height()));
            csTexture.SetNativeTexture(static_cast<uint32_t>(m_texture->nativeTexture().object));
            auto csRenderTarget = graphicsAPI->CreateRenderTarget(CS::Size2U(1, 1));
            csRenderTarget.SetColorAttachment(csTexture);
            m_view->SetRenderTarget(csRenderTarget);

            auto _renderTarget = dynamic_cast<QRhiTextureRenderTarget*>(renderTarget());
            if (_renderTarget != nullptr) {
                _renderTarget->setFlags(_renderTarget->flags() | QRhiTextureRenderTarget::PreserveColorContents);
            }

            m_renderPass = std::make_unique<QBlitRenderPass>(rhi(), _renderTarget->renderPassDescriptor());
            m_renderPass->SetSrcTexture(m_texture.get(), m_sampler.get());

            auto vertices = m_renderPass->getVertices();
            batch->uploadStaticBuffer(vertices.first, vertices.second);

            cb->resourceUpdate(batch);
        }
    }
}

void QuickRenderer::render(QRhiCommandBuffer* cb)
{
    auto renderModule = CS::Engine::Instance().GetModule<CS::RenderModule>();
    if (renderModule.has_value()) {
        renderModule.value()->Render();
    }

    m_renderPass->Submit(cb, renderTarget());
    update();
}

void QuickRenderer::synchronize(QQuickRhiItem* item)
{
    auto viewSize = item->size().toSize();
    if (viewSize != m_texture->pixelSize()) {
        // m_texture->destroy();
        // m_texture->setPixelSize(viewSize);
        // m_texture->create();
        // m_renderPass->SetSrcTexture(m_texture.get(), m_sampler.get());

        if (m_view != nullptr) {
            auto renderTaget = m_view->GetRenderTarget();
            renderTaget.SetSize(CS::Size2U(viewSize.width(), viewSize.height()));
            renderTaget.Build();
        }
    }
}

void QuickRenderer::initializeGraphics() {}

void QuickRenderer::destroyGraphics() {}

} // namespace CSEditor
