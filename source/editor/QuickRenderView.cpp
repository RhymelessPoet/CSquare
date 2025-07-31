#include "QuickRenderView.h"
#include "Engine.h"
#include "GLRendererBuilder.h"
#include "QuickRenderer.h"
#include "RenderModule.h"
#include "View.h"
#include "graphics/GraphicsAPI.h"

namespace CSEditor
{

QuickRenderView::QuickRenderView()
{
    m_renderModule = CS::Engine::Instance().GetModule<CS::RenderModule>();
    if (m_renderModule.has_value()) {
        m_view = m_renderModule.value()->CreateView();

        CS::GLRendererBuilder rendererBuilder;
        m_renderModule.value()->CreateRenderer(rendererBuilder);

        auto graphicsAPI = m_renderModule.value()->GetGraphicsAPI(m_view);

        auto csTexture = graphicsAPI->CreateTexture();

        auto csRenderTarget = graphicsAPI->CreateRenderTarget(CS::Size2U(1, 1));
        csRenderTarget.SetColorAttachment(csTexture);
        m_view->SetRenderTarget(csRenderTarget);
    }
}

QuickRenderView::~QuickRenderView() noexcept {}

QQuickRhiItemRenderer* QuickRenderView::createRenderer()
{
    if (!m_renderModule.has_value() || !m_view) {
        return nullptr;
    }
    auto graphicsAPI = m_renderModule.value()->GetGraphicsAPI(m_view);

    return new QuickRenderer(graphicsAPI, m_view->GetRenderTarget());
}

} // namespace CSEditor
