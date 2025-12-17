#include "MeshRenderer.h"
#include "Camera.h"
#include "asset/BuiltInShaders.h"
#include "geometry/GeometryNode.h"
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

void MeshRenderer::OnUpdate() {}

void MeshRenderer::OnRender(RenderContext& context)
{
    for (const auto& node : m_geometryNodes) {
        render(context, node);
    }
}

void MeshRenderer::AddGeometryNode(std::shared_ptr<GeometryNode> node)
{
    m_geometryNodes.push_back(std::move(node));
}

std::shared_ptr<GeometryNode> MeshRenderer::GetGeometryNode(uint32_t index) const
{
    if (index < m_geometryNodes.size()) {
        return m_geometryNodes[index];
    }
    return std::shared_ptr<GeometryNode>();
}

void MeshRenderer::render(RenderContext& context, std::shared_ptr<GeometryNode> node)
{
    auto mesh = node->GetMesh();
    auto material = node->GetMaterial();
    auto graphicsAPI = context.GetGraphicsAPI();
    if (!m_vertexBuffer.IsValid()) {
        auto vertexBufferSize = mesh->GetVertexBuffer(0u).GetByteSize();
        m_vertexBuffer = graphicsAPI->CreateVertexBuffer(vertexBufferSize);
    }
    if (!m_indexBuffer.IsValid()) {
        auto indexBufferSize = mesh->GetIndices<std::byte>().size();
        m_indexBuffer = graphicsAPI->CreateIndexBuffer(indexBufferSize);
    }

    if (!m_inputAssembly.IsValid()) {
        m_inputAssembly = graphicsAPI->CreateInputAssembly();
        auto vertexInputLayout = node->GetVertexInputLayout();
        uint32_t offset = 0u;
        for (uint32_t binding = 0u; binding < vertexInputLayout->MaxBindings; ++binding) {
            const auto& vertexBinding = vertexInputLayout->GetBinding(binding);
            if (vertexBinding.has_value()) {
                m_inputAssembly.SetVertexInput(binding, m_vertexBuffer, offset);
                offset += vertexBinding.value().GetStride() * mesh->GetVertexCount();
            }
        }
        m_inputAssembly.SetIndexBuffer(m_indexBuffer, uint32_t{});

        m_inputAssembly.SetVertexInputLayout(vertexInputLayout);
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

        auto noError = material->SetUniformValue(std::string_view("model"), modelMatrix.ToStdVector());
    }

    if (m_meshDirty) {
        auto vertexData = mesh->GetVertexBufferView(0u);
        auto indexData = mesh->GetIndices<std::byte>();
        m_vertexBuffer.UpdateData(vertexData.data(), vertexData.size());
        m_indexBuffer.UpdateData(indexData.data(), indexData.size());
        m_meshDirty = false;
    }

    auto& materialCompiler = context.GetMaterialCompiler();

    material->Apply(materialCompiler);

    materialCompiler.SetVertexInputLayout(node->GetVertexInputLayout());
    auto pipeline = materialCompiler.GetPipeline(*material->GetMaterial());
    auto shaderBindingSet = materialCompiler.GetShaderBindingSet(*material);

    auto commandBuffer = context.GetCommandBuffer();

    commandBuffer.Bind(pipeline)
        .Bind(m_inputAssembly)
        .Bind(shaderBindingSet)
        .DrawIndexed(mesh->GetIndices<uint32_t>().size(), 0u);
}

} // namespace CS
