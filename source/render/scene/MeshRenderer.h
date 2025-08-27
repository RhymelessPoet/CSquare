#pragma once
#include "IRenderable.h"
#include "graphics/GraphicsInputAssembly.h"
#include "graphics/GraphicsPipeline.h"
#include "graphics/IndexBuffer.h"
#include "graphics/ShaderBindingSet.h"
#include "graphics/ShaderBindingSetLayout.h"
#include "graphics/UniformBuffer.h"
#include "graphics/VertexBuffer.h"
#include <memory>

namespace CS
{
class Mesh;
class Shader;
class MeshRenderSystem;

class MeshRenderer : public IRenderable
{
public:
    using SystemType = MeshRenderSystem;
    explicit MeshRenderer(std::shared_ptr<SceneObject> owner);
    ~MeshRenderer() override = default;

    virtual void OnUpdate() override;

    virtual void OnRender(RenderContext& context) override;

    void SetMesh(std::shared_ptr<Mesh> mesh);
    std::shared_ptr<Mesh> GetMesh() const { return m_mesh; }

private:
    std::shared_ptr<Mesh> m_mesh;
    IndexBuffer m_indexBuffer;
    VertexBuffer m_vertexBuffer;
    UniformBuffer m_vpMatrixBuffer;
    GraphicsInputAssembly m_inputAssembly;
    GraphicsPipeline m_pipeline;
    std::shared_ptr<Shader> m_vertShader;
    std::shared_ptr<Shader> m_fragShader;
    ShaderBindingSetLayout m_shaderBindingSetLayout;
    ShaderBindingSet m_shaderBindingSet;
    bool m_meshDirty : 1; // Use bit field for memory efficiency
};
} // namespace CS
