#pragma once
#include "IRenderable.h"
#include "graphics/GraphicsInputAssembly.h"
#include "graphics/IndexBuffer.h"
#include "graphics/VertexBuffer.h"
#include "utils/OrientedBoundingBox.h"
#include <map>
#include <memory>

namespace CS
{
class MeshRenderSystem;
class GeometryNode;

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
    struct GeometryData
    {
        GraphicsInputAssembly inputAssembly;
        uint32_t vertexOffset{0u};
        uint32_t indexOffset{0u};
        bool vertexBufferDirty{true};
        bool indexBufferDirty{true};
    };
    using GeometryDataMap = std::map<const std::shared_ptr<GeometryNode>, std::unique_ptr<GeometryData>>;

    void render(RenderContext& context, std::shared_ptr<GeometryNode> node);
    void
    updateGeometryData(const GeometryDataMap& map, const std::shared_ptr<GeometryNode>& preNode, GeometryData& data);
    std::pair<size_t, size_t> getGeometryDataSize(const std::shared_ptr<GeometryNode>& node) const;
    std::pair<size_t, size_t> getGeometryDataOffset(const GeometryDataMap& map,
                                                    const std::shared_ptr<GeometryNode>& node) const;
    GraphicsInputAssembly& getInputAssembly(GeometryDataMap& map, const std::shared_ptr<GeometryNode>& node);

private:
    std::vector<std::shared_ptr<GeometryNode>> m_geometryNodes;
    GeometryDataMap m_geometryData;
    uint32_t m_vertexBufferSize{0u};
    uint32_t m_indexBufferSize{0u};

    IndexBuffer m_indexBuffer;
    VertexBuffer m_vertexBuffer;
};
} // namespace CS
