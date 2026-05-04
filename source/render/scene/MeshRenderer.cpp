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

#include "SceneObjectEvents.h"
#include "SystemContext.h"
#include "core/event/IEventDispatcher.h"
#include <iostream>

namespace CS
{

MeshRenderer::MeshRenderer(std::shared_ptr<SceneObject> owner) : IRenderable(std::move(owner)) {}

void MeshRenderer::OnUpdate(SystemContext& context)
{
    auto eventDispatcher = context.GetEventDispatcher();
    GeometryDataMap newGeometryData;
    std::shared_ptr<GeometryNode> preNode;
    for (const auto& node : m_geometryNodes) {
        auto it = m_geometryData.find(node);
        if (it != m_geometryData.end()) {
            newGeometryData[node] = std::move(it->second);
        } else {
            newGeometryData[node] = std::make_unique<GeometryData>();
            updateGeometryData(newGeometryData, preNode, *newGeometryData[node]);
            eventDispatcher->PushEvent<NewGeometryNode>(this, node);
        }
        preNode = node;
    }
    m_geometryData = std::move(newGeometryData);
    if (m_geometryNodes.empty()) {
        m_vertexBufferSize = 0u;
        m_indexBufferSize = 0u;
    } else {
        auto lastNode = m_geometryNodes.back();
        auto [vertexBufferSize, indexBufferSize] = getGeometryDataSize(lastNode);
        auto [vertexBufferOffset, indexBufferOffset] = getGeometryDataOffset(m_geometryData, lastNode);
        m_vertexBufferSize = vertexBufferOffset + vertexBufferSize;
        m_indexBufferSize = indexBufferOffset + indexBufferSize;
    }
}

void MeshRenderer::OnRender(RenderContext& context)
{
    auto graphicsAPI = context.GetGraphicsAPI();
    if (!m_vertexBuffer.IsValid()) {
        m_vertexBuffer = graphicsAPI->CreateVertexBuffer(m_vertexBufferSize);
    }
    if (!m_indexBuffer.IsValid()) {
        m_indexBuffer = graphicsAPI->CreateIndexBuffer(m_indexBufferSize);
    }
    if (!m_vertexBuffer.IsBuild()) {
        m_vertexBuffer.Build();
    }
    if (!m_indexBuffer.IsBuild()) {
        m_indexBuffer.Build();
    }
    for (const auto& node : m_geometryNodes) {

        auto& inputAssembly = getInputAssembly(m_geometryData, node);
        if (!inputAssembly.IsValid()) {
            inputAssembly = graphicsAPI->CreateInputAssembly();
        }

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

std::vector<OrientedBoundingBox> MeshRenderer::GetWorldBoundingBoxes()
{
    std::vector<OrientedBoundingBox> boxes;
    auto& trans = transform();
    for (const auto& node : m_geometryNodes) {
        auto mesh = node->GetMesh();
        auto box = trans.Trans(mesh->GetAABB());
        boxes.push_back(box);
    }
    return boxes;
}

OrientedBoundingBox MeshRenderer::GetWorldBoundingBox()
{
    AABB aabb;
    for (const auto& node : m_geometryNodes) {
        auto mesh = node->GetMesh();
        aabb.Include(mesh->GetAABB());
    }
    return transform().Trans(aabb);
}

void MeshRenderer::render(RenderContext& context, std::shared_ptr<GeometryNode> node)
{
    auto mesh = node->GetMesh();
    auto material = node->GetMaterial(context.GetTargetViewType());
    if (material == nullptr) {
        return;
    }

    auto& inputAssembly = getInputAssembly(m_geometryData, node);
    auto [vertexBufferSize, indexBufferSize] = getGeometryDataSize(node);
    auto [vertexBufferOffset, indexBufferOffset] = getGeometryDataOffset(m_geometryData, node);

    if (!inputAssembly.IsBuild()) {
        auto vertexInputLayout = node->GetVertexInputLayout();
        uint32_t offset = vertexBufferOffset;
        for (uint32_t binding = 0u; binding < vertexInputLayout->MaxBindings; ++binding) {
            const auto& vertexBinding = vertexInputLayout->GetBinding(binding);
            if (vertexBinding.has_value()) {
                inputAssembly.SetVertexInput(binding, m_vertexBuffer, offset);
                offset += vertexBinding.value().GetStride() * mesh->GetVertexCount();
            }
        }
        inputAssembly.SetIndexBuffer(m_indexBuffer, uint32_t{});
        inputAssembly.SetVertexInputLayout(vertexInputLayout);

        auto vertexData = mesh->GetVertexBufferView(0u);
        auto indexData = mesh->GetIndices<std::byte>();
        m_vertexBuffer.UpdateData(vertexData, vertexBufferOffset);
        m_indexBuffer.UpdateData(indexData, indexBufferOffset);
    }

    auto transform = owner()->GetComponent<Transform>();

    if (transform != nullptr) {
        const auto modelMatrix = transform->GetWorldMatrix().Transposed();

        auto noError = material->SetUniformValue(std::string_view("model"), modelMatrix.ToStdVector());
    }

    auto& materialCompiler = context.GetMaterialCompiler();

    material->Apply(materialCompiler);

    materialCompiler.SetVertexInputLayout(node->GetVertexInputLayout());
    auto pipeline = materialCompiler.GetPipeline(*material->GetMaterial());
    auto shaderBindingSet = materialCompiler.GetShaderBindingSet(*material);

    auto commandBuffer = context.GetCommandBuffer();

    commandBuffer.Bind(pipeline)
        .Bind(inputAssembly)
        .Bind(shaderBindingSet)
        .DrawIndexed(mesh->GetIndices<uint32_t>().size(), indexBufferOffset);
}

void MeshRenderer::updateGeometryData(const GeometryDataMap& map,
                                      const std::shared_ptr<GeometryNode>& preNode,
                                      GeometryData& data)
{
    uint32_t vertexOffset = 0u;
    uint32_t indexOffset = 0u;
    if (preNode) {
        auto [vertexBufferSize, indexBufferSize] = getGeometryDataSize(preNode);
        auto [preVertexOffset, preIndexOffset] = getGeometryDataOffset(map, preNode);
        vertexOffset = preVertexOffset + vertexBufferSize;
        indexOffset = preIndexOffset + indexBufferSize;
    }
    if (data.vertexOffset != vertexOffset) {
        data.vertexOffset = vertexOffset;
        data.vertexBufferDirty = true;
    }
    if (data.indexOffset != indexOffset) {
        data.indexOffset = indexOffset;
        data.indexBufferDirty = true;
    }
}

std::pair<size_t, size_t> MeshRenderer::getGeometryDataSize(const std::shared_ptr<GeometryNode>& node) const
{
    auto mesh = node->GetMesh();
    return std::pair<size_t, size_t>(mesh->GetVertexBuffer(0u).GetByteSize(), mesh->GetIndices<std::byte>().size());
}

std::pair<size_t, size_t> MeshRenderer::getGeometryDataOffset(const GeometryDataMap& map,
                                                              const std::shared_ptr<GeometryNode>& node) const
{
    auto it = map.find(node);
    if (it != map.end()) {
        return std::pair<size_t, size_t>(it->second->vertexOffset, it->second->indexOffset);
    }
    return std::pair<size_t, size_t>(0u, 0u);
}

GraphicsInputAssembly& MeshRenderer::getInputAssembly(GeometryDataMap& map, const std::shared_ptr<GeometryNode>& node)
{
    return map[node]->inputAssembly;
}

} // namespace CS
