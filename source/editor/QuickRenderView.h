#include <QQuickRhiItem>
#include <memory>
namespace CS
{
class View;
class Scene;
class GraphicsAPI;
class RenderModule;
class CameraManipulator;
class IRenderSample;

} // namespace CS
namespace CSEditor
{

class QuickRenderer;

class QuickRenderView : public QQuickRhiItem
{
public:
    struct MouseState
    {
        Qt::MouseButtons pressedButtons{Qt::NoButton};
        // Qt::Key
        QPoint pressPos;
    };
    QuickRenderView(/* args */);
    ~QuickRenderView() noexcept;

    virtual QQuickRhiItemRenderer* createRenderer() override;
    std::shared_ptr<CS::View> GetView() const { return m_view; }
    void FitToScene(bool reCompute = false);

protected:
    virtual void mousePressEvent(QMouseEvent* event) override;
    virtual void mouseReleaseEvent(QMouseEvent* event) override;
    virtual void mouseMoveEvent(QMouseEvent* event) override;
    virtual void wheelEvent(QWheelEvent* event) override;
    virtual void keyPressEvent(QKeyEvent* event) override;
    virtual void keyReleaseEvent(QKeyEvent* event) override;
    virtual void geometryChange(const QRectF& newGeometry, const QRectF& oldGeometry) override;

    void onSampleChange();

private:
    MouseState m_mouseState;
    std::unique_ptr<QTimer> m_timer{nullptr};

    QuickRenderer* m_renderer{nullptr};
    std::shared_ptr<CS::View> m_view;
    std::unique_ptr<CS::IRenderSample> m_sample;
    std::optional<CS::RenderModule*> m_renderModule;
    std::unique_ptr<CS::CameraManipulator> m_cameraManipulator;
};

} // namespace CSEditor
