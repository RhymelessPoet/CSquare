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
layout(location = 4) in vec2 in_texcoord;

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
layout(location = 1) out vec3 world_position;
layout(location = 2) out vec2 tex_coord;

void main()
{
    vec4 model_position = model * vec4(in_position, 1.0);
    gl_Position = projection * view * model_position;

    world_position = model_position.xyz;
    normal = transpose(mat3(model)) * in_normal;
    tex_coord = in_texcoord;
}

)";

static inline constexpr std::string_view PBR_FS = R"(
#version 450 core

// 输入变量
layout(location = 0) in vec3 normal;
layout(location = 1) in vec3 world_position;  // 补充世界空间位置(PBR必需)
layout(location = 2) in vec2 tex_coord;       // 纹理坐标(如果需要纹理采样)

// PBR 材质参数
layout(std140, binding = 2) uniform PBR
{
    vec4 base_color;      // 基础色 (RGB) + 透明度 (A)
    vec4 diffuse_color;   // 漫反射系数(PBR中通常融合到base_color)
    vec4 emission_color;  // 自发光颜色 (RGB) + 强度 (A)
    float metallic;       // 金属度 [0,1]
    float roughness;      // 粗糙度 [0,1]
    float shininess;      // 兼容传统高光的光泽度(PBR中主要用roughness)
};

// 光源参数(示例：单个方向光)
layout(std140, binding = 3) uniform ImagingParameters
{
    vec3 camera_position;        // 相机世界空间位置
    vec3 light_direction;        // 世界空间光源方向(归一化)
    vec3 light_color;            // 光源颜色
    float light_intensity;       // 光源强度
};

// 输出颜色
layout(location = 0) out vec4 FragColor;

// 数学常量
const float PI = 3.14159265359;

// -------------------------- PBR 核心函数 --------------------------
// 法线分布函数 (Trowbridge-Reitz GGX)
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float nom = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / max(denom, 0.0000001); // 防止除零
}

// 几何遮挡函数 (Schlick-GGX)
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float nom = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

// 几何阴影函数 (Smith)
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

// 菲涅尔方程 (Schlick)
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

void main()
{
    // 1. 基础向量归一化
    vec3 N = normalize(normal);                              // 法向量
    vec3 V = normalize(camera_position - world_position);    // 视线方向
    vec3 L = normalize(light_direction);                     // 光源方向
    vec3 H = normalize(V + L);                               // 半程向量

    // 2. 基础材质参数处理
    vec3 albedo = pow(base_color.rgb, vec3(2.2));     // 伽马校正
    float metallic = clamp(metallic, 0.0, 1.0);       // 金属度限制
    float roughness = clamp(roughness, 0.001, 1.0);   // 粗糙度限制(防止除零)
    float ao = 1.0;                                   // 环境遮挡(可扩展)

    // 3. 菲涅尔基值 F0 (非金属=0.04,金属=反照率)
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);

    // 4. 计算PBR各项系数
    float NDF = DistributionGGX(N, H, roughness);       // 法线分布
    float G = GeometrySmith(N, V, L, roughness);        // 几何阴影
    vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);   // 菲涅尔

    // 5. 镜面反射和漫反射计算
    vec3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0);
    vec3 specular = numerator / max(denominator, 0.0000001);

    // 镜面/漫反射权重 (kS=镜面,kD=漫反射)
    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;  // 金属无漫反射

    // 6. 漫反射贡献
    float NdotL = max(dot(N, L), 0.0);
    vec3 diffuse = (kD * albedo / PI) * NdotL;

    // 7. 总直接光照
    vec3 radiance = light_color * light_intensity;
    vec3 directLight = (diffuse + specular) * radiance;

    // 8. 环境光照 (简化版IBL,可替换为CubeMap采样)
    vec3 ambient = vec3(0.03) * albedo * ao;

    // 9. 自发光贡献
    vec3 emission = emission_color.rgb * emission_color.a;

    // 10. 最终颜色计算
    vec3 finalColor = ambient + directLight + emission;

    // 11. 色调映射 + 伽马校正
    finalColor = finalColor / (finalColor + vec3(1.0));  // Reinhard色调映射
    finalColor = pow(finalColor, vec3(1.0/2.2));         // 反伽马校正

    // 输出最终颜色(包含透明度)
    FragColor = vec4(finalColor, base_color.a);
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
    auto binding2 = ShaderBinding{2u, ShaderStage::Fragment, ShaderBinding::Type::UniformBuffer};
    binding2.SetLayout({{"base_color", 4 * sizeof(float)},
                        {"diffuse_color", 4 * sizeof(float)},
                        {"emission_color", 4 * sizeof(float)},
                        {"metallic", sizeof(float)},
                        {"roughness", sizeof(float)},
                        {"shininess", sizeof(float)}});
    auto binding3 = ShaderBinding{3u, ShaderStage::Fragment, ShaderBinding::Type::UniformBuffer};
    binding3.SetLayout({{"camera_position", 3 * sizeof(float)},
                        {"light_direction", 3 * sizeof(float)},
                        {"light_color", 3 * sizeof(float)},
                        {"light_intensity", sizeof(float)}});
    fragShader->AddBinding(binding2);
    fragShader->AddBinding(binding3);

    m_fragmentShaders["PBR_FS"] = fragShader;
}

} // namespace CS
