#include <QQuickRhiItem>

namespace CSEditor
{

class QuickRenderer;

class QuickRenderView : public QQuickRhiItem
{
public:
    QuickRenderView(/* args */);
    ~QuickRenderView() noexcept;

    virtual QQuickRhiItemRenderer* createRenderer() override;

private:
    QuickRenderer* m_renderer{nullptr};
};

} // namespace CSEditor
