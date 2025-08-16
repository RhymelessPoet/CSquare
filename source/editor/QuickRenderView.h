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

private:
    QuickRenderer* m_renderer{nullptr};
    std::shared_ptr<CS::View> m_view;
    std::shared_ptr<CS::Scene> m_scene;
    std::optional<CS::RenderModule*> m_renderModule;
};

} // namespace CSEditor
