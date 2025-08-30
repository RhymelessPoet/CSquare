#include "MeshRenderer.h"
#include "Camera.h"
#include "assets/BuiltInShaders.h"
#include "geometry/Mesh.h"
#include "graphics/GraphicsAPI.h"
#include "graphics/GraphicsShaderStage.h"
#include "graphics/VertexInputLayout.h"
#include "materials/Shader.h"
#include "renderer/RenderContext.h"

#include "materials/Material.h"
#include "materials/MaterialComputer.h"

namespace CS
{

MeshRenderer::MeshRenderer(std::shared_ptr<SceneObject> owner) : IRenderable(std::move(owner)) {}

void MeshRenderer::OnUpdate()
{
    if (m_mesh == nullptr || m_material == nullptr) {
        return; // No mesh to render
    }
}

void MeshRenderer::OnRender(RenderContext& context)
{
    auto graphicsAPI = context.GetGraphicsAPI();
    if (!m_vertexBuffer.IsValid()) {
        auto vertexBufferSize = m_mesh->GetVertexData().size() * sizeof(float);
        m_vertexBuffer = graphicsAPI->CreateVertexBuffer(vertexBufferSize);
    }
    if (!m_indexBuffer.IsValid()) {
        auto indexBufferSize = m_mesh->GetIndexData().size() * sizeof(uint32_t);
        m_indexBuffer = graphicsAPI->CreateIndexBuffer(indexBufferSize);
    }
    if (!m_vpMatrixBuffer.IsValid()) {
        m_vpMatrixBuffer = graphicsAPI->CreateUniformBuffer(sizeof(float) * 32);
    }

    if (!m_inputAssembly.IsValid()) {
        m_inputAssembly = graphicsAPI->CreateInputAssembly();
        m_inputAssembly.SetVertexInput(0u, m_vertexBuffer, 0u);
        m_inputAssembly.SetIndexBuffer(m_indexBuffer, uint32_t{});

        m_inputAssembly.SetVertexInputLayout(m_vertexInputLayout);
    }

    if (!m_shaderBindingSetLayout.IsValid()) {
        m_shaderBindingSetLayout = graphicsAPI->CreateShaderBindingSetLayout();
        m_shaderBindingSetLayout.AddBinding({0u, ShaderStage::Vertex, ShaderBinding::Type::UniformBuffer});
    }

    if (!m_shaderBindingSet.IsValid()) {
        m_shaderBindingSet = graphicsAPI->CreateShaderBindingSet(m_shaderBindingSetLayout);
        m_shaderBindingSet.BindUniformBuffer(0u, m_vpMatrixBuffer, 0u, sizeof(float) * 32);
    }

    if (!m_vertexBuffer.IsBuild()) {
        m_vertexBuffer.Build();
    }
    if (!m_indexBuffer.IsBuild()) {
        m_indexBuffer.Build();
    }
    if (!m_vpMatrixBuffer.IsBuild()) {
        m_vpMatrixBuffer.Build();
    }
    {
        auto camera = context.GetCamera();
        std::array<float, 32> vpMatrix;
        const auto viewMatrix = camera->GetViewMatrix().Transposed();
        const auto projectionMatrix = camera->GetProjectionMatrix().Transposed();
        const auto& arr1 = viewMatrix.Data();
        const auto& arr2 = projectionMatrix.Data();
        std::copy(arr1.begin(), arr1.end(), vpMatrix.begin());
        std::copy(arr2.begin(), arr2.end(), vpMatrix.begin() + arr1.size());
        m_vpMatrixBuffer.UpdateData(vpMatrix.data(), vpMatrix.size() * sizeof(float));
    }

    if (m_meshDirty) {
        auto vertexData = m_mesh->GetVertexData();
        auto indexData = m_mesh->GetIndexData();
        m_vertexBuffer.UpdateData(vertexData.data(), vertexData.size() * sizeof(float));
        m_indexBuffer.UpdateData(indexData.data(), indexData.size() * sizeof(uint32_t));
        m_meshDirty = false;
    }

    auto commandBuffer = context.GetCommandBuffer();

    auto& materialComputer = context.GetMaterialComputer();
    materialComputer.SetVertexInputLayout(m_vertexInputLayout);
    auto pipeline = materialComputer.GetPipeline(*m_material);

    commandBuffer.Bind(pipeline).Bind(m_inputAssembly).Bind(m_shaderBindingSet).DrawIndexed(3u, 0u);
}

void MeshRenderer::SetMesh(std::shared_ptr<Mesh> mesh)
{
    m_mesh = std::move(mesh);
    m_meshDirty = true; // Mark the mesh as dirty to trigger updates in rendering
}

void MeshRenderer::SetVertexInputLayout(std::shared_ptr<VertexInputLayout> layout)
{
    m_vertexInputLayout = std::move(layout);
}

} // namespace CS
