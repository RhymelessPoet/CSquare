#include <QQuickRhiItem>

namespace CSEditor
{

class CSQuickRenderer;

class CSQuickRenderView : public QQuickRhiItem
{
public:
    CSQuickRenderView(/* args */);
    ~CSQuickRenderView() noexcept;

    virtual QQuickRhiItemRenderer* createRenderer() override;

private:
    CSQuickRenderer* m_renderer{nullptr};
};

} // namespace CSEditor
