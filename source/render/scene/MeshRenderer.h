#pragma once
#include "IRenderable.h"
#include "graphics/GraphicsInputAssembly.h"
#include "graphics/IndexBuffer.h"
#include "graphics/VertexBuffer.h"
#include "utils/OrientedBoundingBox.h"
#include <array>
#include <memory>

namespace CS
{
class Mesh;
class Shader;
class MeshRenderSystem;
class MaterialInstance;
class MaterialCompiler;
class VertexInputLayout;
class GeometryNode;
class OrientedBoundingBox;

class MeshRenderer : public IRenderable
{
public:
    using SystemType = MeshRenderSystem;
    explicit MeshRenderer(std::shared_ptr<SceneObject> owner);
    ~MeshRenderer() override = default;

    virtual void OnUpdate() override;

    virtual void OnRender(RenderContext& context) override;

    void AddGeometryNode(std::shared_ptr<GeometryNode> node);
    std::shared_ptr<GeometryNode> GetGeometryNode(uint32_t index) const;

    std::vector<OrientedBoundingBox> GetWorldBoundingBoxes();
    OrientedBoundingBox GetWorldBoundingBox();

private:
    void render(RenderContext& context, std::shared_ptr<GeometryNode> node);

private:
    std::vector<std::shared_ptr<GeometryNode>> m_geometryNodes;

    IndexBuffer m_indexBuffer;
    VertexBuffer m_vertexBuffer;

    GraphicsInputAssembly m_inputAssembly;

    bool m_meshDirty : 1; // Use bit field for memory efficiency
};
} // namespace CS
