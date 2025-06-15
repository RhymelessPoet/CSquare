#include "RenderTarget.h"
#include <QQuickRhiItem>

namespace CS
{
class View;
}

class QRhiSampler;

namespace CSEditor
{

class QBlitRenderPass;

class QuickRenderer : public QQuickRhiItemRenderer
{
public:
    QuickRenderer(/* args */);
    ~QuickRenderer();

    virtual void initialize(QRhiCommandBuffer* cb) override;
    virtual void render(QRhiCommandBuffer* cb) override;
    virtual void synchronize(QQuickRhiItem* item) override;

private:
    void initializeGraphics();
    void destroyGraphics();

private:
    std::shared_ptr<CS::View> m_view;
    std::unique_ptr<QBlitRenderPass> m_renderPass;
    std::unique_ptr<QRhiTexture> m_texture;
    std::unique_ptr<QRhiSampler> m_sampler;
};

} // namespace CSEditor
