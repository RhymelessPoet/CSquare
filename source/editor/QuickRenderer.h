#include <QQuickRhiItem>

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
    QRhiTexture* m_texture{nullptr};
};

} // namespace CSEditor
