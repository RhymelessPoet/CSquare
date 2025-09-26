#include "MeshRenderer.h"
#include "Camera.h"
#include "asset/BuiltInShaders.h"
#include "geometry/Mesh.h"
#include "graphics/GraphicsAPI.h"
#include "graphics/GraphicsShaderStage.h"
#include "graphics/VertexInputLayout.h"
#include "materials/Shader.h"
#include "renderer/RenderContext.h"

#include "materials/MaterialInstance.h"
#include "renderer/MaterialCompiler.h"

#include "SceneObject.h"
#include "Transform.h"

#include <iostream>

namespace CS
{

MeshRenderer::MeshRenderer(std::shared_ptr<SceneObject> owner) : IRenderable(std::move(owner)) {}

void MeshRenderer::OnUpdate()
{
    if (m_mesh == nullptr || m_material == nullptr || m_vertexInputLayout == nullptr) {
        return; // No mesh to render
    }
}

void MeshRenderer::OnRender(RenderContext& context)
{
    if (m_vertexInputLayout == nullptr) {
        return;
    }
    auto graphicsAPI = context.GetGraphicsAPI();
    if (!m_vertexBuffer.IsValid()) {
        auto vertexBufferSize = m_mesh->GetVertexData().size() * sizeof(float);
        m_vertexBuffer = graphicsAPI->CreateVertexBuffer(vertexBufferSize);
    }
    if (!m_indexBuffer.IsValid()) {
        auto indexBufferSize = m_mesh->GetIndexData().size() * sizeof(uint32_t);
        m_indexBuffer = graphicsAPI->CreateIndexBuffer(indexBufferSize);
    }

    if (!m_inputAssembly.IsValid()) {
        m_inputAssembly = graphicsAPI->CreateInputAssembly();
        m_inputAssembly.SetVertexInput(0u, m_vertexBuffer, 0u);
        m_inputAssembly.SetIndexBuffer(m_indexBuffer, uint32_t{});

        m_inputAssembly.SetVertexInputLayout(m_vertexInputLayout);
    }

    if (!m_vertexBuffer.IsBuild()) {
        m_vertexBuffer.Build();
    }
    if (!m_indexBuffer.IsBuild()) {
        m_indexBuffer.Build();
    }

    auto transform = owner()->GetComponent<Transform>();

    if (transform != nullptr) {
        const auto modelMatrix = transform->GetWorldMatrix().Transposed();

        auto noError = m_material->SetUniformValue(std::string_view("model"), modelMatrix.ToStdVector());
    }

    if (m_meshDirty) {
        auto vertexData = m_mesh->GetVertexData();
        auto indexData = m_mesh->GetIndexData();
        m_vertexBuffer.UpdateData(vertexData.data(), vertexData.size() * sizeof(float));
        m_indexBuffer.UpdateData(indexData.data(), indexData.size() * sizeof(uint32_t));
        m_meshDirty = false;
    }

    auto& materialCompiler = context.GetMaterialCompiler();

    m_material->Apply(materialCompiler);

    materialCompiler.SetVertexInputLayout(m_vertexInputLayout);
    auto pipeline = materialCompiler.GetPipeline(*m_material->GetMaterial());
    auto shaderBindingSet = materialCompiler.GetShaderBindingSet(*m_material);

    auto commandBuffer = context.GetCommandBuffer();

    commandBuffer.Bind(pipeline)
        .Bind(m_inputAssembly)
        .Bind(shaderBindingSet)
        .DrawIndexed(m_mesh->GetIndexData().size(), 0u);
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
