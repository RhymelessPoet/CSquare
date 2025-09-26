#include "PanoramicSky.h"
#include "MeshRenderSystem.h"
#include "MeshRenderer.h"
#include "Scene.h"
#include "SceneObject.h"
#include "Transform.h"
#include "TransformSystem.h"
#include "asset/BuiltInShaders.h"
#include "geometry/Mesh.h"
#include "graphics/VertexInputLayout.h"
#include "materials/ImageTexture.h"
#include "materials/Material.h"
#include "materials/Shader.h"

namespace CS
{

// clang-format off
static const std::vector<float> vertices = {-1.0f, 1.0f, 1.0, 1.0f,
                                            -1.0f, -1.0f, 1.0, -1.0f};
// clang-format on

PanoramicSky::PanoramicSky(std::shared_ptr<Scene> scene) : m_scene(std::move(scene))
{
    m_sceneObject = m_scene->CreateSceneObject();

    m_scene->GetSystem<MeshRenderSystem>().CreateComponent<MeshRenderer>(m_sceneObject);
    m_scene->GetSystem<TransformSystem>().CreateComponent<Transform>(m_sceneObject);

    auto& meshRenderer = GetComponent<MeshRenderer>(m_sceneObject);

    auto vertShader = std::make_shared<Shader>(std::string(Panoramic_Sky_VS), ShaderStage::Vertex);
    auto binding0 = ShaderBinding{0u, ShaderStage::Vertex, ShaderBinding::Type::UniformBuffer};
    binding0.SetLayout({{"view", 16 * sizeof(float)}, {"projection", 16 * sizeof(float)}});

    vertShader->AddBinding(binding0);

    auto binding1 = ShaderBinding{1u, ShaderStage::Fragment, ShaderBinding::Type::SampledTexture};
    auto materialTexture = std::make_unique<ImageTexture>();
    materialTexture->SetAddressModeUV(AddressMode::Repeat, AddressMode::Repeat);
    binding1.SetTexture(ShaderBindingTexture("hdr_texture", std::move(materialTexture)));
    auto fragShader = std::make_shared<Shader>(std::string(Panoramic_Sky_FS), ShaderStage::Fragment);
    fragShader->AddBinding(binding1);

    // clang-format off
    auto material = Material::Builder()
                    .Begin()
                    .AddInputAttribute(0u, VertexInputFormat::Float2)
                    .AddShader(vertShader)
                    .AddShader(fragShader)
                    .End();
    // clang-format on

    m_material = material->CreateInstance();
    meshRenderer.SetMaterial(m_material);

    auto mesh = std::make_shared<Mesh>(vertices, std::vector<uint32_t>{0u, 1u, 2u, 1u, 3u, 2u});
    meshRenderer.SetMesh(mesh);

    auto vertexInputLayout = std::make_shared<VertexInputLayout>();

    vertexInputLayout->SetBinding(0u, VertexInputBinding{2u * sizeof(float), VertexInputRate::PerVertex})
        .SetAttribute(0u, VertexInputAttribute{0u, VertexInputFormat::Float2, 0u});
    meshRenderer.SetVertexInputLayout(vertexInputLayout);
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
