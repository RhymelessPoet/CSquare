#include "QuickRenderView.h"
#include "Engine.h"
#include "GLRendererBuilder.h"
#include "MeshRenderSystem.h"
#include "MeshRenderer.h"
#include "QuickRenderer.h"
#include "RenderModule.h"
#include "Scene.h"
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

        m_scene = std::make_shared<CS::Scene>();
        m_scene->AddSystem<CS::MeshRenderSystem>(graphicsAPI);
        m_scene->GetSystem<CS::MeshRenderSystem>().CreateComponent<CS::MeshRenderer>(m_scene->GetRoot());

        m_view->SetScene(m_scene);
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
