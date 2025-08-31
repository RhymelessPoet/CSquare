#pragma once
#include "IRenderable.h"
#include "graphics/GraphicsInputAssembly.h"
#include "graphics/GraphicsPipeline.h"
#include "graphics/IndexBuffer.h"
#include "graphics/ShaderBindingSet.h"
#include "graphics/ShaderBindingSetLayout.h"
#include "graphics/UniformBuffer.h"
#include "graphics/VertexBuffer.h"
#include <array>
#include <memory>


namespace CS
{
class Mesh;
class Shader;
class MeshRenderSystem;
class Material;
class MaterialComputer;
class VertexInputLayout;

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

    void SetMaterial(std::shared_ptr<Material> material) { m_material = std::move(material); }
    const std::shared_ptr<const Material>& GetMaterial() const { return m_material; }

    void SetVertexInputLayout(std::shared_ptr<VertexInputLayout> layout);

private:
    std::shared_ptr<VertexInputLayout> m_vertexInputLayout;
    std::shared_ptr<Mesh> m_mesh;
    std::shared_ptr<Material> m_material;

    IndexBuffer m_indexBuffer;
    VertexBuffer m_vertexBuffer;
    UniformBuffer m_vpMatrixBuffer;
    UniformBuffer m_modelMatrixBuffer;
    GraphicsInputAssembly m_inputAssembly;

    std::array<float, 16> m_modelData;

    std::shared_ptr<Shader> m_vertShader;
    std::shared_ptr<Shader> m_fragShader;
    ShaderBindingSetLayout m_shaderBindingSetLayout;
    ShaderBindingSet m_shaderBindingSet;
    bool m_meshDirty : 1; // Use bit field for memory efficiency
};
} // namespace CS
