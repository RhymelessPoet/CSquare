#include "RenderTarget.h"
#include <QQuickRhiItem>

namespace CS
{
class GraphicsAPI;
} // namespace CS

class QRhiSampler;

namespace CSEditor
{

class QBlitRenderPass;

class QuickRenderer : public QQuickRhiItemRenderer
{
public:
    QuickRenderer(std::shared_ptr<CS::GraphicsAPI> graphicsAPI, CS::RenderTarget renderTarget);
    ~QuickRenderer();

    virtual void initialize(QRhiCommandBuffer* cb) override;
    virtual void render(QRhiCommandBuffer* cb) override;
    virtual void synchronize(QQuickRhiItem* item) override;

private:
    void initializeGraphics();
    void destroyGraphics();

private:
    bool m_initialized{false};
    std::shared_ptr<CS::GraphicsAPI> m_graphicsAPI;
    CS::RenderTarget m_renderTarget;
    std::unique_ptr<QBlitRenderPass> m_renderPass;
    std::unique_ptr<QRhiTexture> m_texture;
    std::unique_ptr<QRhiSampler> m_sampler;
};

} // namespace CSEditor
