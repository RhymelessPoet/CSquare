#include "RenderTarget.h"
#include <QQuickRhiItem>

namespace CS
{
class View;
}

namespace CSEditor
{

class QuickRenderer : public QQuickRhiItemRenderer
{
public:
    QuickRenderer(/* args */);
    ~QuickRenderer();

    virtual void initialize(QRhiCommandBuffer* cb) override;
    virtual void render(QRhiCommandBuffer* cb) override;
    virtual void synchronize(QQuickRhiItem* fbo) override;

private:
    std::shared_ptr<CS::View> m_view;
    QRhiTexture* m_texture{nullptr};
};

} // namespace CSEditor
