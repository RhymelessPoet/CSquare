#include "BuiltInShaders.h"
#include "materials/ImageTexture.h"
#include "materials/Shader.h"

static inline constexpr std::string_view Panoramic_Sky_VS = R"(
#version 450 core
layout(location = 0) in vec2 position;
layout(location = 0) out vec3 world_position;

layout(std140, binding = 0) uniform VPMatrix
{
    mat4 view;
    mat4 projection;
};

void main()
{
    gl_Position = vec4(position.xy, -1.0, 1.0);

    float near = projection[3][2] / (projection[2][2] - 1.0);
    float right = near / projection[0][0];
    float top = near / projection[1][1];

    world_position = transpose(mat3(view)) * vec3(position.x * right, position.y * top, -near);
}

)";

static inline constexpr std::string_view Panoramic_Sky_FS = R"(
#version 450 core

const float PI = 3.14159265359;

layout(location = 0) in vec3 world_position;
layout(binding = 1) uniform sampler2D hdr_texture;

out vec4 FragColor;

void main()
{
    vec3 direction = normalize(world_position);

    float v = acos(direction.y) / PI;
    float u = 0.5 * (atan(direction.x / direction.z) / PI + 0.5);
    u += step(direction.z, 0.0) * 0.5;

    vec3 hdrColor = texture(hdr_texture, vec2(u, v)).rgb;
    vec3 color = hdrColor / (hdrColor + vec3(1.0));
    FragColor = vec4(color, 1.0f);
}

)";

static inline constexpr std::string_view PBR_VS = R"(
#version 450 core
layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec3 in_tagent;
layout(location = 3) in vec3 in_bitagent;
layout(location = 4) in vec2 in_texCoord;

layout(std140, binding = 0) uniform VPMatrix
{
    mat4 view;
    mat4 projection;
};

layout(std140, binding = 1) uniform MMatrix
{
    mat4 model;
};

layout(location = 0) out vec3 normal;

void main()
{
    gl_Position = projection * view * model * vec4(in_position, 1.0);
    normal = transpose(mat3(model)) * in_normal;
}

)";

static inline constexpr std::string_view PBR_FS = R"(
#version 450 core

layout(location = 0) in vec3 normal;

layout(std140, binding = 2) uniform PBR
{
    vec4 base_color;
    vec4 diffuse_color;
    vec4 emission_color;
    float metallic;
    float roughness;
    float shininess;
};

layout(location = 0) out vec4 FragColor;

void main()
{
    FragColor = base_color * vec4(abs(normalize(normal)), 1.0f);
}

)";

namespace CS
{
BuiltInShaders::BuiltInShaders()
{
    createPanoramicSkyShader();
    createPBRShader();
}

std::shared_ptr<Shader> BuiltInShaders::GetVertexShader(std::string_view name)
{
    auto itr = m_vertexShaders.find(std::string(name));
    if (itr != m_vertexShaders.end()) {
        return itr->second;
    }
    return std::shared_ptr<Shader>();
}

std::shared_ptr<Shader> BuiltInShaders::GetFragmentShader(std::string_view name)
{
    auto itr = m_fragmentShaders.find(std::string(name));
    if (itr != m_fragmentShaders.end()) {
        return itr->second;
    }
    return std::shared_ptr<Shader>();
}

void BuiltInShaders::createPanoramicSkyShader()
{
    auto vertShader = std::make_shared<Shader>(std::string(Panoramic_Sky_VS), ShaderStage::Vertex);
    auto binding0 = ShaderBinding{0u, ShaderStage::Vertex, ShaderBinding::Type::UniformBuffer};
    binding0.SetLayout({{"view", 16 * sizeof(float)}, {"projection", 16 * sizeof(float)}});
    vertShader->AddBinding(binding0);
    m_vertexShaders["PanoramicSky_VS"] = vertShader;

    auto binding1 = ShaderBinding{1u, ShaderStage::Fragment, ShaderBinding::Type::SampledTexture};
    auto materialTexture = std::make_unique<ImageTexture>();
    materialTexture->SetAddressModeUV(AddressMode::Repeat, AddressMode::Repeat);
    binding1.SetTexture(ShaderBindingTexture("hdr_texture", std::move(materialTexture)));

    auto fragShader = std::make_shared<Shader>(std::string(Panoramic_Sky_FS), ShaderStage::Fragment);
    fragShader->AddBinding(binding1);
    m_fragmentShaders["PanoramicSky_FS"] = fragShader;
}

void BuiltInShaders::createPBRShader()
{
    auto vertShader = std::make_shared<Shader>(std::string(PBR_VS), ShaderStage::Vertex);
    auto binding0 = ShaderBinding{0u, ShaderStage::Vertex, ShaderBinding::Type::UniformBuffer};
    binding0.SetLayout({{"view", 16 * sizeof(float)}, {"projection", 16 * sizeof(float)}});
    auto binding1 = ShaderBinding{1u, ShaderStage::Vertex, ShaderBinding::Type::UniformBuffer};
    binding1.SetLayout({{"model", 16 * sizeof(float)}});
    vertShader->AddBinding(binding0);
    vertShader->AddBinding(binding1);
    m_vertexShaders["PBR_VS"] = vertShader;

    auto fragShader = std::make_shared<Shader>(std::string(PBR_FS), ShaderStage::Fragment);
    auto binding3 = ShaderBinding{2u, ShaderStage::Fragment, ShaderBinding::Type::UniformBuffer};
    binding3.SetLayout({{"base_color", 4 * sizeof(float)},
                        {"diffuse_color", 4 * sizeof(float)},
                        {"emission_color", 4 * sizeof(float)},
                        {"metallic", sizeof(float)},
                        {"roughness", sizeof(float)},
                        {"shininess", sizeof(float)}});
    fragShader->AddBinding(binding3);
    m_fragmentShaders["PBR_FS"] = fragShader;
}

} // namespace CS
