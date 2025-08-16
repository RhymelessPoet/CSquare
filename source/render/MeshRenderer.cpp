#include "MeshRenderer.h"
#include "GraphicsShaderStage.h"
#include "Mesh.h"
#include "RenderContext.h"
#include "Shader.h"
#include "VertexInputLayout.h"
#include "assets/BuiltInShaders.h"
#include "graphics/GraphicsAPI.h"

namespace CS
{
static const std::vector<float> vertices = {0.0f, 0.5f, 0.0f, 1.0f, 0.0f,  0.0f, -0.5, -0.5f, 0.0f,
                                            0.0f, 1.0f, 0.0f, 0.5f, -0.5f, 0.0f, 0.0f, 0.0f,  1.0f};
MeshRenderer::MeshRenderer(std::shared_ptr<SceneObject> owner) : IRenderable(std::move(owner))
{
    m_vertShader = std::make_shared<Shader>(std::string(VertexShader), ShaderStage::Vertex);
    m_fragShader = std::make_shared<Shader>(std::string(FragmentShader), ShaderStage::Fragment);

    auto mesh = std::make_shared<Mesh>(vertices, std::vector<uint32_t>{0u, 1u, 2u});
    SetMesh(mesh);
}

void MeshRenderer::OnUpdate()
{
    if (m_mesh == nullptr) {
        return; // No mesh to render
    }
    if (!m_vertexBuffer.IsValid()) {
        auto vertexBufferSize = m_mesh->GetVertexData().size() * sizeof(float);
        m_vertexBuffer = getGraphicsAPI()->CreateVertexBuffer(vertexBufferSize);
    }
    if (!m_indexBuffer.IsValid()) {
        auto indexBufferSize = m_mesh->GetIndexData().size() * sizeof(uint32_t);
        m_indexBuffer = getGraphicsAPI()->CreateIndexBuffer(indexBufferSize);
    }

    auto createVertexInputLayout = []() {
        auto vertexInputLayout = std::make_unique<VertexInputLayout>();

        vertexInputLayout->SetBinding(0u, VertexInputBinding{6 * sizeof(float), VertexInputRate::PerVertex})
            .SetAttribute(0u, VertexInputAttribute{0u, VertexInputAttribute::Format::Float3, 0u})
            .SetAttribute(1u, VertexInputAttribute{0u, VertexInputAttribute::Format::Float3, 3 * sizeof(float)});

        return vertexInputLayout;
    };

    if (!m_inputAssembly.IsValid()) {
        m_inputAssembly = getGraphicsAPI()->CreateInputAssembly();
        m_inputAssembly.SetVertexInput(0u, m_vertexBuffer, 0u);
        m_inputAssembly.SetIndexBuffer(m_indexBuffer, uint32_t{});

        m_inputAssembly.SetVertexInputLayout(createVertexInputLayout());
    }

    if (!m_pipeline.IsValid()) {
        m_pipeline = getGraphicsAPI()->CreatePipeline();

        m_pipeline.SetVertexInputLayout(createVertexInputLayout())
            .SetShaderStage(std::make_unique<GraphicsShaderStage>(m_vertShader))
            .SetShaderStage(std::make_unique<GraphicsShaderStage>(m_fragShader));
    }
}

void MeshRenderer::OnRender(RenderContext& context)
{
    if (!m_vertexBuffer.IsBuild()) {
        m_vertexBuffer.Build();
    }
    if (!m_indexBuffer.IsBuild()) {
        m_indexBuffer.Build();
    }
    if (m_meshDirty) {
        auto vertexData = m_mesh->GetVertexData();
        auto indexData = m_mesh->GetIndexData();
        m_vertexBuffer.UpdateData(vertexData.data(), vertexData.size() * sizeof(float));
        m_indexBuffer.UpdateData(indexData.data(), indexData.size() * sizeof(uint32_t));
        m_meshDirty = false;
    }
    if (!m_inputAssembly.IsBuild()) {
        m_inputAssembly.Build();
    }
    if (!m_pipeline.IsBuild()) {
        m_pipeline.Build();
    }

    auto cmdBuf = context.GetCommandBuffer();
    // cmdBuf.Bind(m_inputAssembly).Bind(m_pipeline).DrawIndexed(3u, 0u);
    cmdBuf.Bind(m_pipeline).Bind(m_inputAssembly).DrawIndexed(3u, 0u);
}

void MeshRenderer::SetMesh(std::shared_ptr<Mesh> mesh)
{
    m_mesh = std::move(mesh);
    m_meshDirty = true; // Mark the mesh as dirty to trigger updates in rendering
}

} // namespace CS
