#include "QuickRenderView.h"
#include "Camera.h"
#include "CameraSystem.h"
#include "Engine.h"
#include "GLRendererBuilder.h"
#include "MeshRenderSystem.h"
#include "MeshRenderer.h"
#include "QuickRenderer.h"
#include "RenderModule.h"
#include "Scene.h"
#include "TransformSystem.h"
#include "View.h"
#include "base/math/Math.h"
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

        auto csRenderTarget = graphicsAPI->CreateRenderTarget(CS::Size2u(1, 1));
        csRenderTarget.SetColorAttachment(csTexture);
        m_view->SetRenderTarget(csRenderTarget);

        m_scene = std::make_shared<CS::Scene>();
        m_scene->AddSystem<CS::MeshRenderSystem>(graphicsAPI);
        m_scene->AddSystem<CS::CameraSystem>();
        m_scene->AddSystem<CS::TransformSystem>();
        m_scene->GetSystem<CS::MeshRenderSystem>().CreateComponent<CS::MeshRenderer>(m_scene->GetRoot());

        m_view->SetScene(m_scene);

        auto camera = m_view->GetCamera();
        camera->LookAt({0.0f, 0.0f, 3.5f}, {0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f});
        camera->Perspective(CS::Math::AnglesToRadians(45.0f), 1.0f, 0.1f, 100.0f);
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

void QuickRenderView::mousePressEvent(QMouseEvent* event) {}

void QuickRenderView::mouseReleaseEvent(QMouseEvent* event) {}

void QuickRenderView::mouseMoveEvent(QMouseEvent* event) {}

void QuickRenderView::wheelEvent(QWheelEvent* event) {}

void QuickRenderView::keyPressEvent(QKeyEvent* event) {}

void QuickRenderView::keyReleaseEvent(QKeyEvent* event) {}

} // namespace CSEditor
