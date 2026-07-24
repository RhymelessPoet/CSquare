#pragma once
#include <QQuickItem>
#include <memory>
#include <optional>

namespace CS
{
class View;
class Scene;
class GraphicsAPI;
class RenderModule;
class CameraManipulator;

} // namespace CS

class QSGTexture;

namespace CSEditor
{

class QuickRenderView : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(QString projectID WRITE setProjectID)
public:
    struct MouseState
    {
        Qt::MouseButtons pressedButtons{Qt::NoButton};
        QPoint pressPos;
    };
    QuickRenderView();
    ~QuickRenderView() noexcept;

    std::shared_ptr<CS::View> GetView() const { return m_view; }
    Q_INVOKABLE void FitToScene(bool reCompute = false);

    Q_INVOKABLE void setProjectID(const QString& projectID);

signals:
    // Emitted after the engine GL renderer (main View + render target) is
    // fully initialized. QML uses this to kick off asset loading only after
    // the main view exists (see QEditor::beginAssetLoading).
    void engineReady();

protected:
    virtual void mousePressEvent(QMouseEvent* event) override;
    virtual void mouseReleaseEvent(QMouseEvent* event) override;
    virtual void mouseMoveEvent(QMouseEvent* event) override;
    virtual void wheelEvent(QWheelEvent* event) override;
    virtual void keyPressEvent(QKeyEvent* event) override;
    virtual void keyReleaseEvent(QKeyEvent* event) override;
    virtual void geometryChange(const QRectF& newGeometry, const QRectF& oldGeometry) override;

    virtual QSGNode* updatePaintNode(QSGNode* oldNode, UpdatePaintNodeData* data) override;
    virtual void itemChange(ItemChange change, const ItemChangeData& value) override;

private:
    void initializeEngineRenderer();
    void resizeRenderTarget(const QSize& pixelSize);
    uint32_t engineColorTextureId() const;

private:
    MouseState m_mouseState;

    std::shared_ptr<CS::View> m_view;
    std::shared_ptr<CS::GraphicsAPI> m_graphicsAPI;
    std::optional<CS::RenderModule*> m_renderModule;
    std::unique_ptr<CS::CameraManipulator> m_cameraManipulator;

    bool m_engineInitialized{false};
    QSize m_renderPixelSize{0, 0};
    QString m_pendingProjectID;
};

} // namespace CSEditor
