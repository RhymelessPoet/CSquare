#pragma once
#include <rhi/qrhi.h>

class QRhi;
class QRhiCommandBuffer;
class QRhiBuffer;
class QRhiGraphicsPipeline;
class QRhiRenderPassDescriptor;
class QRhiRenderTarget;

namespace CSEditor
{

class QBlitRenderPass
{
public:
    QBlitRenderPass(QRhi* rhi, QRhiRenderPassDescriptor* descriptor);
    ~QBlitRenderPass();

    void SetSrcTexture(QRhiTexture* texture, QRhiSampler* sampler);
    QPair<QRhiBuffer*, float*> getVertices();
    void Submit(QRhiCommandBuffer* commandBuffer, QRhiRenderTarget* renderTarget);

private:
    QRhi* m_rhi{nullptr};
    QRhiCommandBuffer::VertexInput* m_vertexInput{nullptr};
    QRhiGraphicsPipeline* m_pipeline{nullptr};
};

} // namespace CSEditor
