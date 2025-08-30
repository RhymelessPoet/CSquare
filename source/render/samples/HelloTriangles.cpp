#include "HelloTriangles.h"
#include "scene/CameraSystem.h"
#include "scene/MeshRenderSystem.h"
#include "scene/MeshRenderer.h"
#include "scene/Scene.h"
#include "scene/TransformSystem.h"
#include "scene/View.h"

#include "assets/BuiltInShaders.h"
#include "geometry/Mesh.h"
#include "materials/Material.h"
#include "materials/Shader.h"
#include "scene/SceneObject.h"

#include "graphics/VertexInputLayout.h"

namespace CS
{
static const std::vector<float> vertices = {0.0f, 0.5f, 0.0f, 1.0f, 0.0f,  0.0f, -0.5, -0.5f, 0.0f,
                                            0.0f, 1.0f, 0.0f, 0.5f, -0.5f, 0.0f, 0.0f, 0.0f,  1.0f};

void HelloTriangles::Initialize(std::shared_ptr<View> view)
{
    m_scene = std::make_shared<CS::Scene>();
    m_scene->AddSystem<CS::MeshRenderSystem>();
    m_scene->AddSystem<CS::CameraSystem>();
    m_scene->AddSystem<CS::TransformSystem>();

    view->SetScene(m_scene);

    auto root = m_scene->GetRoot();
    m_scene->GetSystem<CS::MeshRenderSystem>().CreateComponent<CS::MeshRenderer>(root);

    auto& component = root->GetComponent<CS::MeshRenderer>();
    auto& meshRender = dynamic_cast<CS::MeshRenderer&>(component);

    auto mesh = std::make_shared<Mesh>(vertices, std::vector<uint32_t>{0u, 1u, 2u});
    meshRender.SetMesh(mesh);

    auto vertShader = std::make_shared<Shader>(std::string(VertexShader), ShaderStage::Vertex);
    auto fragShader = std::make_shared<Shader>(std::string(FragmentShader), ShaderStage::Fragment);

    // clang-format off
    auto material = Material::Builder()
                    .Begin()
                    .AddShader(vertShader)
                    .AddShader(fragShader)
                    .End();
    // clang-format on

    meshRender.SetMaterial(material);

    auto vertexInputLayout = std::make_shared<VertexInputLayout>();

    vertexInputLayout->SetBinding(0u, VertexInputBinding{6 * sizeof(float), VertexInputRate::PerVertex})
        .SetAttribute(0u, VertexInputAttribute{0u, VertexInputFormat::Float3, 0u})
        .SetAttribute(1u, VertexInputAttribute{0u, VertexInputFormat::Float3, 3 * sizeof(float)});

    meshRender.SetVertexInputLayout(vertexInputLayout);
}

} // namespace CS
