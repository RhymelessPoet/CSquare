#include "PanoramicSky.h"
#include "MeshRenderSystem.h"
#include "MeshRenderer.h"
#include "Scene.h"
#include "SceneObject.h"
#include "Transform.h"
#include "TransformSystem.h"
#include "asset/BuiltInShaders.h"
#include "geometry/GeometryNode.h"
#include "graphics/VertexInputLayout.h"
#include "materials/IMaterialConfiguration.h"
#include "materials/ImageTexture.h"
#include "materials/Material.h"
#include "materials/Shader.h"

namespace CS
{

class PanoramicSkyMaterialConfiguration : public IMaterialConfiguration
{
public:
    virtual void Configure(GraphicsPipeline& pipeline) const override { pipeline.SetDepthTest(false); }
};

// clang-format off
static const std::vector<float> vertices = {-1.0f, 1.0f, 1.0, 1.0f,
                                            -1.0f, -1.0f, 1.0, -1.0f};
// clang-format on

PanoramicSky::PanoramicSky(std::shared_ptr<Scene> scene) : m_scene(std::move(scene))
{
    m_sceneObject = m_scene->CreateSceneObject();
    m_sceneObject->SetName("PanoramicSky");

    m_scene->GetSystem<MeshRenderSystem>().CreateComponent<MeshRenderer>(m_sceneObject);
    m_scene->GetSystem<TransformSystem>().CreateComponent<Transform>(m_sceneObject);

    auto& meshRenderer = GetComponent<MeshRenderer>(m_sceneObject);

    // clang-format off
    auto material = Material::Builder()
                    .Begin(std::make_unique<PanoramicSkyMaterialConfiguration>())
                    .AddInputAttribute(0u, VertexInputFormat::Float2)
                    .AddShader(BuiltInShaders::Instance().GetVertexShader("PanoramicSky_VS"))
                    .AddShader(BuiltInShaders::Instance().GetFragmentShader("PanoramicSky_FS"))
                    .End();
    // clang-format on

    m_material = material->CreateInstance();

    auto mesh = Mesh::Builder()
                    .AddVertexBuffer(vertices)
                    .SetVertexCount(4u)
                    .AddAttribute(Mesh::Attribute{"_position", 0u, 8 * sizeof(float), 0u, 2u, DataType::Float32})
                    .SetIndices(Buffer(std::vector<uint32_t>{0u, 1u, 2u, 1u, 3u, 2u}), DataType::UInt32)
                    .Build();

    auto geometryNode = std::make_shared<GeometryNode>(mesh, m_material);
    geometryNode->SetAttributeMap("_position", 0u);
    meshRenderer.AddGeometryNode(geometryNode);
}

PanoramicSky::~PanoramicSky()
{
    m_sceneObject->GetParent()->RemoveChild(m_sceneObject);
}

void PanoramicSky::SetImage(std::shared_ptr<Image> image)
{
    auto instanceTexture = dynamic_cast<ImageTexture*>(m_material->GetInstanceTexture("hdr_texture"));
    if (instanceTexture != nullptr) {
        instanceTexture->SetImage(image);
        return;
    }
    auto texture = m_material->GetDefaultInstance().GetInstanceTexture("hdr_texture");
    if (texture == nullptr) {
        // TODO: log error
        return;
    }
    auto insTextureUPtr = texture->Clone();
    instanceTexture = dynamic_cast<ImageTexture*>(insTextureUPtr.get());
    if (instanceTexture == nullptr) {
        // TODO : log error
        return;
    }

    instanceTexture->SetImage(image);
    auto noError = m_material->SetTexture("hdr_texture", std::move(insTextureUPtr));
    if (!noError) {
        // TODO: log error
    }
}

} // namespace CS
