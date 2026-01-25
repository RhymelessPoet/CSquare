#include "QuickRenderView.h"
#include "Engine.h"
#include "QuickRenderer.h"
#include "RenderModule.h"
#include "base/math/Math.h"
#include "graphics/GLRendererBuilder.h"
#include "graphics/GraphicsAPI.h"
#include "scene/CameraSystem.h"
#include "scene/MeshRenderSystem.h"
#include "scene/SceneObjectComposer.h"
#include "scene/SystemGraph.h"
#include "scene/TransformSystem.h"

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
        renderModule->GetSystemGraph().AddSystem<CS::CameraSystem>();
        renderModule->GetSystemGraph().AddSystem<CS::TransformSystem>();

        m_view = renderModule->GetMainView();

        CS::GLRendererBuilder rendererBuilder;
        renderModule->CreateRenderer(rendererBuilder);

        auto graphicsAPI = renderModule->GetGraphicsAPI(m_view);

        auto csTexture = graphicsAPI->CreateTexture();
        auto depthTexture = graphicsAPI->CreateTexture(CS::TextureFormat::Depth24Stencil8);

        auto csRenderTarget = graphicsAPI->CreateRenderTarget(CS::Size2u(1, 1));
        csRenderTarget.SetColorAttachment(csTexture);
        csRenderTarget.SetDepthStencilAttachment(depthTexture);
        m_view->SetRenderTarget(csRenderTarget);

        m_sample = std::make_unique<CS::SAssetLoad>(renderModule->GetSOComposer());
        // m_sample = std::make_unique<CS::SPanoramicHDRSky>();
        // m_sample = std::make_unique<CS::SHelloTriangles>();

        m_timer = std::make_unique<QTimer>();
        onSampleChange();
    }
    setAcceptedMouseButtons(Qt::AllButtons);
    setAcceptHoverEvents(true);
    setFocus(true);

    m_timer->setInterval(40);
    m_timer->start();
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
    m_cameraManipulator->Zoom(event->angleDelta().y());
}

void QuickRenderView::keyPressEvent(QKeyEvent* event)
{
    auto key = event->key();
    bool dollyNear = key == Qt::Key_W || key == Qt::Key_Up;
    bool dollyFar = key == Qt::Key_S || key == Qt::Key_Down;

    if (dollyNear) {
        if (event->isAutoRepeat()) {
            m_cameraManipulator->Dolly(1);

        } else {
            m_cameraManipulator->Dolly(3);
        }
    }
    if (dollyFar) {
        if (event->isAutoRepeat()) {
            m_cameraManipulator->Dolly(-1);

        } else {
            m_cameraManipulator->Dolly(-3);
        }
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

void QuickRenderView::onSampleChange()
{
    m_sample->Initialize(m_view);
    connect(m_timer.get(), &QTimer::timeout, [this]() { m_sample->OnUpdate(); });

    m_cameraManipulator = std::make_unique<CS::CameraManipulator>(m_view->GetCamera(), CS::Size2u{1080, 720});
    m_cameraManipulator->LookAt({0.0f, 0.0f, 40.0f}, {0.0f, 0.0f, 0.0f});
}

} // namespace CSEditor
