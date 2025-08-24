#include <QQuickRhiItem>
#include <memory>
namespace CS
{
class View;
class Scene;
class GraphicsAPI;
class RenderModule;
} // namespace CS
namespace CSEditor
{

class QuickRenderer;

class QuickRenderView : public QQuickRhiItem
{
public:
    QuickRenderView(/* args */);
    ~QuickRenderView() noexcept;

    virtual QQuickRhiItemRenderer* createRenderer() override;
    std::shared_ptr<CS::View> GetView() const { return m_view; }

protected:
    virtual void mousePressEvent(QMouseEvent* event) override;
    virtual void mouseReleaseEvent(QMouseEvent* event) override;
    virtual void mouseMoveEvent(QMouseEvent* event) override;
    virtual void wheelEvent(QWheelEvent* event) override;
    virtual void keyPressEvent(QKeyEvent* event) override;
    virtual void keyReleaseEvent(QKeyEvent* event) override;

private:
    QuickRenderer* m_renderer{nullptr};
    std::shared_ptr<CS::View> m_view;
    std::shared_ptr<CS::Scene> m_scene;
    std::optional<CS::RenderModule*> m_renderModule;
};

} // namespace CSEditor
