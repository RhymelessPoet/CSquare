#include <QQuickRhiItem>

namespace CSEditor
{

class CSQuickRenderer : public QQuickRhiItemRenderer
{
public:
    CSQuickRenderer(/* args */);
    ~CSQuickRenderer();

    virtual void initialize(QRhiCommandBuffer* cb) override;
    virtual void render(QRhiCommandBuffer* cb) override;
    virtual void synchronize(QQuickRhiItem* fbo) override;

private:
    QRhiTexture* m_texture{nullptr};
};

} // namespace CSEditor
