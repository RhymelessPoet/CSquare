#include "QuickRenderView.h"
#include "Engine.h"
#include "ProjectManager.h"
#include "ProjectModel.h"
#include "RenderModule.h"
#include "base/Logger.h"
#include "base/math/Math.h"
#include "graphics/GLRendererBuilder.h"
#include "graphics/GraphicsAPI.h"
#include "scene/CameraSystem.h"
#include "scene/LightSystem.h"
#include "scene/MeshRenderSystem.h"
#include "scene/Scene.h"
#include "scene/SceneObjectComposer.h"
#include "scene/SystemGraph.h"
#include "scene/TransformSystem.h"
#include "scene/View.h"
#include "utils/CameraManipulator.h"

#include <QOpenGLContext>
#include <QQuickWindow>
#include <QSGRendererInterface>
#include <QSGSimpleTextureNode>
#include <QSGTexture>

#ifdef Q_OS_WIN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

#include <algorithm>

namespace CSEditor
{

QuickRenderView::QuickRenderView()
{
    setFlag(ItemHasContents, true);
    setAcceptedMouseButtons(Qt::AllButtons);
    setAcceptHoverEvents(true);
    // NOTE: intentionally do NOT call setFocus(true) / setFocusPolicy() here.
    // Calling them in the constructor dispatches focus events synchronously
    // during QML instantiation, which under QSG_RENDER_LOOP=basic contributes
    // to the recursive flushWindowSystemEvents() cascade. Focus can be
    // requested from QML with forceActiveFocus() if needed.

    m_renderModule = CS::Engine::Instance().GetModule<CS::RenderModule>();
    if (m_renderModule.has_value()) {
        auto renderModule = m_renderModule.value();
        renderModule->GetSystemGraph().AddSystem<CS::MeshRenderSystem>();
        renderModule->GetSystemGraph().AddSystem<CS::LightSystem>();
        renderModule->GetSystemGraph().AddSystem<CS::CameraSystem>();
        renderModule->GetSystemGraph().AddSystem<CS::TransformSystem>();
    }
}

QuickRenderView::~QuickRenderView() noexcept {}

void QuickRenderView::itemChange(ItemChange change, const ItemChangeData& value)
{
    QQuickItem::itemChange(change, value);
    if (change == ItemSceneChange && window() != nullptr && !m_engineInitialized) {
        // Defer the heavy engine GL initialization (wglMakeCurrent +
        // gladLoadGL + FBO/texture allocation) OUT of Qt's synchronous
        // sceneGraphInitialized signal frame via Qt::QueuedConnection.
        // Running it inline reenters Qt's scene-graph/event pipeline while
        // Qt is still in the middle of flushWindowSystemEvents(), which
        // produces unbounded notify() recursion under QSG_RENDER_LOOP=basic.
        // Because this delays view creation past QML's Component.onCompleted,
        // ProjectModel must tolerate a null main view and scene attachment
        // is completed here (ProjectManager::AttachAllToView) once the
        // renderer becomes available.
        connect(window(), &QQuickWindow::sceneGraphInitialized, this, &QuickRenderView::initializeEngineRenderer,
                Qt::QueuedConnection);
        if (window()->isSceneGraphInitialized()) {
            QMetaObject::invokeMethod(this, &QuickRenderView::initializeEngineRenderer, Qt::QueuedConnection);
        }
    }
}

void QuickRenderView::initializeEngineRenderer()
{
    if (m_engineInitialized || !m_renderModule.has_value() || window() == nullptr) {
        return;
    }

#ifdef Q_OS_WIN
    // Acquire native HWND and the Qt RHI HGLRC for resource sharing.
    HWND hwnd = reinterpret_cast<HWND>(window()->winId());
    HGLRC qtHglrc = nullptr;
    if (auto* ri = window()->rendererInterface()) {
        auto* glCtx =
            static_cast<QOpenGLContext*>(ri->getResource(window(), QSGRendererInterface::OpenGLContextResource));
        if (glCtx != nullptr) {
            if (auto* wgl = glCtx->nativeInterface<QNativeInterface::QWGLContext>()) {
                qtHglrc = wgl->nativeContext();
            }
        }
    }

    auto renderModule = m_renderModule.value();
    CS::GLRendererBuilder rendererBuilder;
    rendererBuilder.SetNativeWindow(reinterpret_cast<void*>(hwnd), reinterpret_cast<void*>(qtHglrc));
    renderModule->CreateRenderer(rendererBuilder);

    m_view = renderModule->GetMainView();

    m_graphicsAPI = renderModule->GetGraphicsAPI(m_view);

    // Switch to the engine's HGLRC for all subsequent GL object creation so
    // that textures/FBOs are owned by our context (and visible to Qt via
    // wglShareLists).
    m_graphicsAPI->MakeContextCurrent();
    m_graphicsAPI->Initialize();

    const QSize initialSize = size().toSize() * window()->devicePixelRatio();
    const CS::Size2u initial{static_cast<uint32_t>(std::max(initialSize.width(), 1)),
                             static_cast<uint32_t>(std::max(initialSize.height(), 1))};

    // Create engine-owned color + depth textures sized to the viewport.
    auto color = m_graphicsAPI->CreateTexture(CS::TextureFormat::RGBA8Unorm, initial);
    auto depth = m_graphicsAPI->CreateTexture(CS::TextureFormat::Depth32, initial);

    auto renderTarget = m_graphicsAPI->CreateRenderTarget(initial);
    renderTarget.SetColorAttachment(color);
    renderTarget.SetDepthAttachment(depth);
    // Eagerly allocate GL storage for the color/depth textures and build the
    // FBO. Qt's QSGOpenGLTexture::fromNative requires a texture that is both
    // named and "specified" (storage-allocated) at the time the QSG frame
    // records it; otherwise qrhigles2.cpp asserts on texD->specified.
    renderTarget.Build();
    m_view->SetRenderTarget(renderTarget);

    m_renderPixelSize = QSize(initial.width, initial.height);

    // Release our context now that setup is done; Qt's render loop will
    // take back ownership for its own scene graph work.
    m_graphicsAPI->DoneContextCurrent();

    m_engineInitialized = true;

    // Attach scenes of any projects that were created while the view did
    // not yet exist (see ProjectModel::ProjectModel's null-guarded path).
    ProjectManager::Instance().AttachAllToView(m_view);

    // Re-apply any projectID that was set by QML bindings before the view
    // became available, so the camera manipulator and initial fit-to-scene
    // happen now.
    if (!m_pendingProjectID.isEmpty()) {
        const QString pending = m_pendingProjectID;
        m_pendingProjectID.clear();
        setProjectID(pending);
    }

    // Grant keyboard focus now that initialization is complete. This runs
    // on the event-loop tick (QueuedConnection), not inside Qt's scene-graph
    // signal frame, so it does not re-enter flushWindowSystemEvents().
    setActiveFocusOnTab(true);
    forceActiveFocus(Qt::OtherFocusReason);

    // Notify QML that the engine renderer is ready. QML triggers asset
    // loading in response so that scene assets are imported only after the
    // main View exists.
    emit engineReady();

    update();
#else
    CS::LogError(CS::BuiltInChannels::Render(), "QuickRenderView: non-Windows platform not supported");
#endif
}

void QuickRenderView::FitToScene(bool reCompute)
{
    if (m_view == nullptr || m_cameraManipulator == nullptr) {
        return;
    }
    const auto& aabb = m_view->GetScene()->GetAABB(reCompute);
    if (aabb.IsValid()) {
        m_cameraManipulator->FitTo(aabb);
    }
}

void QuickRenderView::setProjectID(const QString& projectID)
{
    if (projectID.isEmpty()) {
        return;
    }
    if (m_view == nullptr) {
        // Engine renderer not yet initialized (GL init is queued out of
        // sceneGraphInitialized). Remember the projectID and re-apply it
        // inside initializeEngineRenderer() once m_view is valid.
        m_pendingProjectID = projectID;
        return;
    }
    // Build the camera manipulator now. Do NOT call FitToScene here: at
    // project-load time the asset has not been imported yet and the scene
    // AABB is empty. FitToScene is triggered later from QML once
    // QEditor::sceneAssetLoaded signals that assets are in the scene.
    m_cameraManipulator = std::make_unique<CS::CameraManipulator>(m_view->GetCamera(), CS::Size2u{width(), height()});
}

uint32_t QuickRenderView::engineColorTextureId() const
{
    if (!m_graphicsAPI || !m_view) {
        return 0;
    }
    auto colorTex = m_graphicsAPI->GetColorAttachment(m_view->GetRenderTarget());
    auto native = colorTex.GetNativeTexture();
    if (!native.has_value()) {
        return 0;
    }
    try {
        return std::any_cast<uint32_t>(native);
    } catch (const std::bad_any_cast&) {
        try {
            return static_cast<uint32_t>(std::any_cast<GLuint>(native));
        } catch (...) {
            return 0;
        }
    }
}

void QuickRenderView::resizeRenderTarget(const QSize& pixelSize)
{
    if (!m_view || !m_graphicsAPI) {
        return;
    }
    if (pixelSize.width() <= 0 || pixelSize.height() <= 0) {
        return;
    }
    if (pixelSize == m_renderPixelSize) {
        return;
    }

    const CS::Size2u newSize{static_cast<uint32_t>(pixelSize.width()), static_cast<uint32_t>(pixelSize.height())};

    m_graphicsAPI->MakeContextCurrent();
    auto renderTarget = m_view->GetRenderTarget();
    auto color = m_graphicsAPI->GetColorAttachment(renderTarget);
    color.SetSize(newSize);
    renderTarget.SetSize(newSize);
    renderTarget.Build();
    m_graphicsAPI->DoneContextCurrent();

    m_renderPixelSize = pixelSize;
}

QSGNode* QuickRenderView::updatePaintNode(QSGNode* oldNode, UpdatePaintNodeData* /*data*/)
{
    auto* node = static_cast<QSGSimpleTextureNode*>(oldNode);
    if (!m_engineInitialized) {
        return node;
    }

    const uint32_t glId = engineColorTextureId();
    if (glId == 0) {
        return node;
    }

    if (node == nullptr) {
        node = new QSGSimpleTextureNode();
        node->setFiltering(QSGTexture::Linear);
        node->setOwnsTexture(true);
    }

    const QSize texSize = m_renderPixelSize.isEmpty() ? size().toSize() : m_renderPixelSize;
    const quint64 textureHandle = static_cast<quint64>(glId);
    QSGTexture* sgTex = QNativeInterface::QSGOpenGLTexture::fromNative(static_cast<GLuint>(glId), window(), texSize,
                                                                       QQuickWindow::TextureHasAlphaChannel);
    node->setTexture(sgTex);
    node->setRect(boundingRect());
    // Flip vertically: engine renders with lower-left origin (OpenGL) while
    // Qt's scene graph samples with upper-left origin.
    node->setTextureCoordinatesTransform(QSGSimpleTextureNode::MirrorVertically);

    // Schedule the next frame so the engine keeps producing content.
    update();
    return node;
}

void QuickRenderView::mousePressEvent(QMouseEvent* event)
{
    // Claim keyboard focus so subsequent keyPressEvent reaches this item.
    // Safe here because the press originates from a user input event, not
    // from QML construction, so no flushWindowSystemEvents recursion.
    forceActiveFocus(Qt::MouseFocusReason);

    auto button = event->button();
    m_mouseState.pressedButtons = m_mouseState.pressedButtons | button;
    m_mouseState.pressPos = event->pos();

    if (button == Qt::RightButton && m_cameraManipulator) {
        m_cameraManipulator->BeginRotate();
    }
    event->accept();
}

void QuickRenderView::mouseReleaseEvent(QMouseEvent* event)
{
    m_mouseState.pressedButtons = m_mouseState.pressedButtons ^ event->button();
    if (m_cameraManipulator) {
        m_cameraManipulator->EndRotate();
    }
    event->accept();
}

void QuickRenderView::mouseMoveEvent(QMouseEvent* event)
{
    auto point = event->pos();
    CS::Vector2f delta{point.x() - m_mouseState.pressPos.x(), point.y() - m_mouseState.pressPos.y()};

    if (m_cameraManipulator) {
        if ((m_mouseState.pressedButtons & Qt::RightButton) != 0) {
            m_cameraManipulator->RotateTrack(delta, 3.0f);
        }
        if ((m_mouseState.pressedButtons & Qt::MiddleButton) != 0) {
            m_cameraManipulator->FlyMove(delta, 0.01f);
        }

        if ((m_mouseState.pressedButtons & Qt::MiddleButton) != 0 && (event->modifiers() & Qt::ControlModifier) != 0) {
            m_cameraManipulator->AlongAxisMove(delta, 0.01f);
        }
    }

    m_mouseState.pressPos = point;
}

void QuickRenderView::wheelEvent(QWheelEvent* event)
{
    if (!m_cameraManipulator) {
        return;
    }
    if ((event->modifiers() & Qt::ShiftModifier) != 0) {
        m_cameraManipulator->Zoom(event->angleDelta().y());
    } else {
        m_cameraManipulator->Dolly(event->angleDelta().y(), 0.01f);
    }
}

void QuickRenderView::keyPressEvent(QKeyEvent* event)
{
    if (!m_cameraManipulator) {
        return;
    }
    auto key = event->key();
    bool dollyNear = key == Qt::Key_W || key == Qt::Key_Up;
    bool dollyFar = key == Qt::Key_S || key == Qt::Key_Down;

    if (dollyNear) {
        m_cameraManipulator->Dolly(event->isAutoRepeat() ? 5 : 10);
    }
    if (dollyFar) {
        m_cameraManipulator->Dolly(event->isAutoRepeat() ? -5 : -10);
    }
    if (key == Qt::Key_F) {
        FitToScene(false);
    }
    if ((key & Qt::Key_F) != 0 && (event->modifiers() & Qt::ShiftModifier) != 0) {
        FitToScene(true);
    }
}

void QuickRenderView::keyReleaseEvent(QKeyEvent* /*event*/) {}

void QuickRenderView::geometryChange(const QRectF& newGeometry, const QRectF& oldGeometry)
{
    QQuickItem::geometryChange(newGeometry, oldGeometry);
    if (m_cameraManipulator != nullptr && newGeometry.height() > 0.0) {
        m_cameraManipulator->SetViewport(CS::Size2u{newGeometry.width(), newGeometry.height()});
    }

    if (m_engineInitialized && window() != nullptr) {
        const qreal dpr = window()->devicePixelRatio();
        QSize pixelSize(std::max(1, static_cast<int>(newGeometry.width() * dpr)),
                        std::max(1, static_cast<int>(newGeometry.height() * dpr)));
        resizeRenderTarget(pixelSize);
    }
    update();
}

} // namespace CSEditor
