#include "SHelloTriangles.h"
#include "scene/CameraSystem.h"
#include "scene/MeshRenderSystem.h"
#include "scene/MeshRenderer.h"
#include "scene/Scene.h"
#include "scene/Transform.h"
#include "scene/TransformSystem.h"
#include "scene/View.h"

#include "geometry/GeometryNode.h"
#include "materials/Material.h"
#include "materials/Shader.h"
#include "scene/SceneObject.h"

#include "graphics/VertexInputLayout.h"

#include "base/math/Math.h"

static constexpr std::string_view VertexShader = R"(
#version 450 core
layout(location = 0) in vec3 _position;
layout(location = 1) in vec3 _color;
layout(location = 0) out vec3 color;

layout(std140, binding = 0) uniform VPMatrix
{
    mat4 view;
    mat4 projection;
};

layout(std140, binding = 1) uniform MMatrix
{
    mat4 model;
};

void main()
{
    mat4 mvpMatrix = projection * view * model;
    gl_Position = mvpMatrix * vec4(_position.x, _position.y, _position.z, 1.0);
    color = _color;
}

)";

static constexpr std::string_view FragmentShader = R"(
#version 450 core

layout(location = 0) in vec3 color;
out vec4 FragColor;

void main()
{
    FragColor = vec4(color, 1.0f);
}

)";

namespace CS
{
// clang-format off
static const std::vector<float> vertices = {0.0f, 0.5f, 0.0f, 1.0f, 0.0f,  0.0f,
                                            -0.5, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f,
                                            0.5f, -0.5f, 0.0f, 0.0f, 0.0f,  1.0f};
// clang-format on

void SHelloTriangles::Initialize(std::shared_ptr<View> view)
{
    m_scene = std::make_shared<CS::Scene>();
    m_scene->AddSystem<CS::MeshRenderSystem>();
    m_scene->AddSystem<CS::CameraSystem>();
    m_scene->AddSystem<CS::TransformSystem>();

    view->SetScene(m_scene);

    auto mesh =
        Mesh::Builder()
            .AddVertexBuffer(vertices)
            .SetVertexCount(3u)
            .AddAttribute(Mesh::Attribute{"_position", 0u, 15 * sizeof(float), 0u, 3u, DataType::Float32})
            .AddAttribute(Mesh::Attribute{"_color", 3 * sizeof(float), 18 * sizeof(float), 0u, 3u, DataType::Float32})
            .SetIndices(Buffer(std::vector<uint32_t>{0u, 1u, 2u}), DataType::UInt32)
            .Build();

    auto vertShader = std::make_shared<Shader>(std::string(VertexShader), ShaderStage::Vertex);

    auto binding0 = ShaderBinding{0u, ShaderStage::Vertex, ShaderBinding::Type::UniformBuffer};
    binding0.SetLayout({{"view", 16 * sizeof(float)}, {"projection", 16 * sizeof(float)}});
    auto binding1 = ShaderBinding{1u, ShaderStage::Vertex, ShaderBinding::Type::UniformBuffer};
    binding1.SetLayout({{"model", 16 * sizeof(float)}});

    vertShader->AddBinding(binding0);
    vertShader->AddBinding(binding1);

    auto fragShader = std::make_shared<Shader>(std::string(FragmentShader), ShaderStage::Fragment);

    // clang-format off
    auto material = Material::Builder()
                    .Begin()
                    .AddInputAttribute(0u, VertexInputFormat::Float3)
                    .AddInputAttribute(1u, VertexInputFormat::Float3)
                    .AddShader(vertShader)
                    .AddShader(fragShader)
                    .End();
    // clang-format on

    auto& meshRenderSystem = m_scene->GetSystem<CS::MeshRenderSystem>();
    auto& transformSystem = m_scene->GetSystem<CS::TransformSystem>();

    m_groupRoot = m_scene->CreateSceneObject();
    meshRenderSystem.CreateComponent<CS::MeshRenderer>(m_groupRoot);
    transformSystem.CreateComponent<CS::Transform>(m_groupRoot);

    auto& meshRender = CS::GetComponent<CS::MeshRenderer>(m_groupRoot);

    auto materialInstance = material->CreateInstance();

    auto geometryNode = std::make_shared<GeometryNode>(mesh, materialInstance);
    geometryNode->SetAttributeMap("_position", 0u);
    geometryNode->SetAttributeMap("_color", 1u);

    meshRender.AddGeometryNode(geometryNode);

    constexpr uint32_t tranglesCount = 2500u;
    auto positions = RandomPositions(tranglesCount, {-10.0f, -10.0f, -10.0f}, {10.0f, 10.0f, 10.0f});

    for (uint32_t index = 0u; index < tranglesCount; ++index) {
        auto child = m_scene->CreateSceneObject(m_groupRoot);

        meshRenderSystem.CreateComponent<CS::MeshRenderer>(child);
        transformSystem.CreateComponent<CS::Transform>(child);

        auto& _meshRender = CS::GetComponent<CS::MeshRenderer>(child);

        auto _geometryNode = std::make_shared<GeometryNode>(mesh, material->CreateInstance());
        _geometryNode->SetAttributeMap("_position", 0u);
        _geometryNode->SetAttributeMap("_color", 1u);

        _meshRender.AddGeometryNode(_geometryNode);

        auto& _transform = CS::GetComponent<CS::Transform>(child);
        _transform.SetPosition(positions[index]);
    }
}

void SHelloTriangles::OnUpdate()
{
    static float Angle = 0.0f;

    Angle += 1.0f;
    while (Angle >= 360.0f) {
        Angle -= 360.0f;
    }

    auto& _transform = GetComponent<Transform>(m_groupRoot);

    _transform.SetRotation({0.0f, 0.0f, Math::AngleToRadian(Angle)});
}

} // namespace CS
