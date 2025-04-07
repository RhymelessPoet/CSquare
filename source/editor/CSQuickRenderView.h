#include <QQuickFramebufferObject>

namespace CSEditor
{

class CSQuickRenderer;

class CSQuickRenderView : public QQuickFramebufferObject
{
public:
    CSQuickRenderView(/* args */);
    ~CSQuickRenderView() noexcept;

    virtual Renderer* createRenderer() const override;

private:
    CSQuickRenderer* m_renderer{nullptr};
};

} // namespace CSEditor
