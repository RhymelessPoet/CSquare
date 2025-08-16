#include "QBlitRenderPass.h"
#include <QFile>

namespace CSEditor
{
// static float vertices[] = {-1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f};
static float vertices[] = {-1.0f, -1.0f, 0.0f, 0.0f, 1.0f, -1.0f, 1.0f, 0.0f,
                           -1.0f, 1.0f,  0.0f, 1.0f, 1.0f, 1.0f,  1.0f, 1.0f};

QShader GetShader(const QString& name)
{
    QFile f(name);
    if (f.open(QIODevice::ReadOnly)) {
        return QShader::fromSerialized(f.readAll());
    }
    return QShader();
}

QBlitRenderPass::QBlitRenderPass(QRhi* rhi, QRhiRenderPassDescriptor* descriptor) : m_rhi(rhi)
{
    auto vbo = m_rhi->newBuffer(QRhiBuffer::Immutable, QRhiBuffer::VertexBuffer, sizeof(vertices));
    vbo->create();
    m_vertexInput = new QRhiCommandBuffer::VertexInput(vbo, 0u);

    QShader vs = GetShader(":/shaders/full_screen.vert.qsb");
    QShader fs = GetShader(":/shaders/full_screen_blit.frag.qsb");

    QRhiShaderResourceBindings* srb = rhi->newShaderResourceBindings();
    // srb->create();

    QRhiVertexInputLayout inputLayout;
    inputLayout.setBindings({{4 * sizeof(float)}});
    inputLayout.setAttributes(
        {{0, 0, QRhiVertexInputAttribute::Float2, 0}, {0, 1, QRhiVertexInputAttribute::Float2, 2 * sizeof(float)}});

    // inputLayout.setBindings({{2 * sizeof(float)}});
    // inputLayout.setAttributes({{0, 0, QRhiVertexInputAttribute::Float2, 0}});

    m_pipeline = m_rhi->newGraphicsPipeline();
    m_pipeline->setTopology(QRhiGraphicsPipeline::TriangleStrip);
    m_pipeline->setVertexInputLayout(inputLayout);
    m_pipeline->setShaderStages({{QRhiShaderStage::Vertex, vs}, {QRhiShaderStage::Fragment, fs}});
    m_pipeline->setShaderResourceBindings(srb);
    m_pipeline->setRenderPassDescriptor(descriptor);
    m_pipeline->setDepthTest(false);
    m_pipeline->create();
}

QBlitRenderPass::~QBlitRenderPass() {}

void QBlitRenderPass::SetSrcTexture(QRhiTexture* texture, QRhiSampler* sampler)
{
    m_pipeline->shaderResourceBindings()->destroy();
    m_pipeline->shaderResourceBindings()->setBindings(
        {QRhiShaderResourceBinding::sampledTexture(0, QRhiShaderResourceBinding::FragmentStage, texture, sampler)});
    m_pipeline->shaderResourceBindings()->create();
}

QPair<QRhiBuffer*, float*> QBlitRenderPass::getVertices()
{
    return {m_vertexInput->first, vertices};
}

void QBlitRenderPass::Submit(QRhiCommandBuffer* commandBuffer, QRhiRenderTarget* renderTarget)
{
    auto outputSize = renderTarget->pixelSize();
    const QColor clearColor = QColor::fromRgbF(0.7f, 0.3f, 0.2f, 1.0f);

    commandBuffer->beginPass(renderTarget, clearColor, {1.0f, 0}, nullptr);
    commandBuffer->setGraphicsPipeline(m_pipeline);
    commandBuffer->setShaderResources(m_pipeline->shaderResourceBindings());
    commandBuffer->setViewport(QRhiViewport(0, 0, outputSize.width(), outputSize.height()));
    commandBuffer->setVertexInput(0, 1, m_vertexInput);
    commandBuffer->draw(4);
    commandBuffer->endPass();
}

} // namespace CSEditor