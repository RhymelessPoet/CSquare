#include "QuickRenderView.h"
#include "Engine.h"
#include "QuickRenderer.h"
#include "RenderModule.h"
#include "base/math/Math.h"
#include "graphics/GLRendererBuilder.h"
#include "graphics/GraphicsAPI.h"
#include "scene/CameraSystem.h"
#include "scene/LightSystem.h"
#include "scene/MeshRenderSystem.h"
#include "scene/SceneObjectComposer.h"
#include "scene/SystemGraph.h"
#include "scene/TransformSystem.h"

#include "scene/Scene.h"
#include "scene/View.h"

#include "utils/CameraManipulator.h"

#include "samples/SAssetLoad.h"
#include "samples/SHelloTriangles.h"
#include "samples/SPanoramicHDRSky.h"

#include <QTimer>

namespace CSEditor
{

QuickRenderView::QuickRenderView()
{
    m_renderModule = CS::Engine::Instance().GetModule<CS::RenderModule>();
    if (m_renderModule.has_value()) {
        auto renderModule = m_renderModule.value();
        renderModule->GetSystemGraph().AddSystem<CS::MeshRenderSystem>();
        renderModule->GetSystemGraph().AddSystem<CS::LightSystem>();
        renderModule->GetSystemGraph().AddSystem<CS::CameraSystem>();
        renderModule->GetSystemGraph().AddSystem<CS::TransformSystem>();

        CS::GLRendererBuilder rendererBuilder;
        renderModule->CreateRenderer(rendererBuilder);

        m_view = renderModule->GetMainView();

        auto graphicsAPI = renderModule->GetGraphicsAPI(m_view);

        auto csTexture = graphicsAPI->CreateTexture();
        auto depthTexture = graphicsAPI->CreateTexture(CS::TextureFormat::Depth32);

        auto csRenderTarget = graphicsAPI->CreateRenderTarget(CS::Size2u(1, 1));
        csRenderTarget.SetColorAttachment(csTexture);
        csRenderTarget.SetDepthAttachment(depthTexture);
        m_view->SetRenderTarget(csRenderTarget);
    }
    setAcceptedMouseButtons(Qt::AllButtons);
    setAcceptHoverEvents(true);
    setFocus(true);
    setFocusPolicy(Qt::StrongFocus);
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

void QuickRenderView::FitToScene(bool reCompute)
{
    if (m_view == nullptr) {
        return;
    }
    const auto& aabb = m_view->GetScene()->GetAABB(reCompute);
    if (aabb.IsValid()) {
        m_cameraManipulator->FitTo(aabb);
    }
}

void QuickRenderView::setProjectID(const QString& projectID)
{
    if (!projectID.isEmpty()) {
        m_cameraManipulator =
            std::make_unique<CS::CameraManipulator>(m_view->GetCamera(), CS::Size2u{width(), height()});
        FitToScene(true);
    }
}

void QuickRenderView::mousePressEvent(QMouseEvent* event)
{
    auto button = event->button();
    m_mouseState.pressedButtons = m_mouseState.pressedButtons | button;
    m_mouseState.pressPos = event->pos();

    if (button == Qt::RightButton) {
        m_cameraManipulator->BeginRotate();
    }
    event->accept();
}

void QuickRenderView::mouseReleaseEvent(QMouseEvent* event)
{
    m_mouseState.pressedButtons = m_mouseState.pressedButtons ^ event->button();
    m_cameraManipulator->EndRotate();
    event->accept();
}

void QuickRenderView::mouseMoveEvent(QMouseEvent* event)
{
    auto point = event->pos();
    CS::Vector2f delta{point.x() - m_mouseState.pressPos.x(), point.y() - m_mouseState.pressPos.y()};

    if ((m_mouseState.pressedButtons & Qt::RightButton) != 0) {
        m_cameraManipulator->RotateTrack(delta, 3.0f);
    }
    if ((m_mouseState.pressedButtons & Qt::MiddleButton) != 0) {
        m_cameraManipulator->FlyMove(delta, 0.01f);
    }

    if ((m_mouseState.pressedButtons & Qt::MiddleButton) != 0 && (event->modifiers() & Qt::ControlModifier) != 0) {
        m_cameraManipulator->AlongAxisMove(delta, 0.01f);
    }

    m_mouseState.pressPos = point;
}

void QuickRenderView::wheelEvent(QWheelEvent* event)
{
    if ((event->modifiers() & Qt::ShiftModifier) != 0) {
        m_cameraManipulator->Zoom(event->angleDelta().y());
    } else {
        m_cameraManipulator->Dolly(event->angleDelta().y(), 0.01f);
    }
}

void QuickRenderView::keyPressEvent(QKeyEvent* event)
{
    auto key = event->key();
    bool dollyNear = key == Qt::Key_W || key == Qt::Key_Up;
    bool dollyFar = key == Qt::Key_S || key == Qt::Key_Down;

    if (dollyNear) {
        if (event->isAutoRepeat()) {
            m_cameraManipulator->Dolly(5);

        } else {
            m_cameraManipulator->Dolly(10);
        }
    }
    if (dollyFar) {
        if (event->isAutoRepeat()) {
            m_cameraManipulator->Dolly(-5);

        } else {
            m_cameraManipulator->Dolly(-10);
        }
    }
    if (key == Qt::Key_F) {
        FitToScene(false);
    }
    if ((key & Qt::Key_F) != 0 && (event->modifiers() & Qt::ShiftModifier) != 0) {
        FitToScene(true);
    }
}

void QuickRenderView::keyReleaseEvent(QKeyEvent* event) {}

void QuickRenderView::geometryChange(const QRectF& newGeometry, const QRectF& oldGeometry)
{
    QQuickRhiItem::geometryChange(newGeometry, oldGeometry);
    if (m_cameraManipulator != nullptr && newGeometry.height() > 0.0) {
        m_cameraManipulator->SetViewport(CS::Size2u{newGeometry.width(), newGeometry.height()});
    }
}

} // namespace CSEditor
