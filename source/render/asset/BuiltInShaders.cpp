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
layout(location = 2) in vec3 in_tangent;
layout(location = 3) in vec3 in_bitangent;
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
layout(location = 3) out vec3 tangent;
layout(location = 4) out vec3 bitangent;

void main()
{
    vec4 model_position = model * vec4(in_position, 1.0);
    gl_Position = projection * view * model_position;

    world_position = model_position.xyz;
    mat3 normalMatrix = transpose(inverse(mat3(model)));
    normal = normalMatrix * in_normal;
    tangent = normalMatrix * in_tangent;
    bitangent = normalMatrix * in_bitangent;
    tex_coord = in_texcoord;
}

)";

static inline constexpr std::string_view PBR_FS = R"(
#version 450 core

// 输入变量
layout(location = 0) in vec3 normal;
layout(location = 1) in vec3 world_position;
layout(location = 2) in vec2 tex_coord;
layout(location = 3) in vec3 tangent;
layout(location = 4) in vec3 bitangent;

// PBR 材质参数（双模式兼容）
layout(std140, binding = 2) uniform PBR
{
    vec4 base_color;          // 基础色（金属工作流, 高光工作流）
    vec4 diffuse_color;       // 漫反射颜色（高光工作流）
    vec4 specular_color;      // 高光颜色（高光工作流）
    vec4 emission_color;      // 自发光颜色 + 强度
    float metallic;           // 金属度（金属工作流）
    float roughness;          // 粗糙度（金属工作流）
    float glossiness;         // 光泽度（高光工作流）

    float normal_scale;       // 法线贴图强度

    unsigned int use_spec_gloss;     // 【模式开关】true=高光/光泽度, false=金属/粗糙度
    unsigned int use_emission_color_map;

    unsigned int use_base_color_map;  // 金属工作流, 高光工作流
    unsigned int use_metallic_map;
    unsigned int use_roughness_map;

    unsigned int use_specular_map;    // 高光纹理
    unsigned int use_glossiness_map;  // 光泽度纹理

    unsigned int use_normal_map;
};

// 光源参数
layout(std140, binding = 3) uniform ImagingParameters
{
    vec3 camera_position;
    vec3 light_direction;
    vec3 light_color;
    float light_intensity;
};

// 纹理绑定
layout(binding = 4) uniform sampler2D base_color_map;
layout(binding = 5) uniform sampler2D emission_color_map;
layout(binding = 6) uniform sampler2D metallic_map;
layout(binding = 7) uniform sampler2D roughness_map;
layout(binding = 8) uniform sampler2D normal_map;
layout(binding = 9) uniform sampler2D specular_map;       // 新增：高光纹理
layout(binding = 10) uniform sampler2D glossiness_map;   // 新增：光泽度纹理

// 输出颜色
layout(location = 0) out vec4 FragColor;

const float PI = 3.14159265359;

struct MetallicRoughnessParameters
{
    vec3 albedo;
    float metallic;
    float roughness;

    vec3 normal;
    vec3 view_direction;
};

vec4 GetBaseColor()
{
    if (use_base_color_map != 0) {
        vec4 tex_color = texture(base_color_map, tex_coord);
        return vec4(tex_color.rgb, base_color.a);
    } else {
        return base_color;
    }
}

vec4 GetDiffuseColor()
{
    if (use_base_color_map != 0) {
        return texture(base_color_map, tex_coord);
    } else {
        return diffuse_color;
    }
}

vec3 GetSpecularColor()
{
    if (use_specular_map != 0) {
        return texture(specular_map, tex_coord).rgb;
    } else {
        return specular_color.rgb;
    }
}

float GetMetallic()
{
    if (use_metallic_map != 0) {
        return texture(metallic_map, tex_coord).r;
    } else {
        return metallic;
    }
}

float GetRoughness()
{
    if (use_roughness_map != 0) {
        return texture(roughness_map, tex_coord).r;
    } else {
        return roughness;
    }
}

float GetGlossiness()
{
    if (use_glossiness_map != 0) {
        return texture(glossiness_map, tex_coord).r;
    } else {
        return glossiness;
    }
}

vec4 GetEmissionColor()
{
    if (use_emission_color_map != 0) {
        vec4 tex_color = texture(emission_color_map, tex_coord);
        return vec4(tex_color.rgb, emission_color.a);
    } else {
        return emission_color;
    }
}

vec3 GetNormal()
{
    if(use_normal_map == 0)
        return normalize(normal);

    vec3 tangent_normal = texture(normal_map, tex_coord).rgb;
    float height = tangent_normal.b;
    tangent_normal = normalize(tangent_normal * 2.0 - 1.0);
    tangent_normal.xy *= normal_scale;
    tangent_normal.z = mix(height, tangent_normal.z, normal_scale);

    vec3 T = normalize(tangent);
    vec3 N = normalize(normal);

    T = normalize(T - dot(T, N) * N);
    vec3 B = normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

    return normalize(TBN * tangent_normal);
}

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float nom = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / max(denom, 0.0000001);
}

// 几何遮挡函数 (Schlick-GGX)
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float nom = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / max(denom, 0.0000001);
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

vec3 PBRShade(MetallicRoughnessParameters params)
{
    vec3 N = params.normal;
    vec3 V = params.view_direction;
    vec3 L = normalize(light_direction);
    vec3 H = normalize(V + L);

    vec3 F0 = mix(vec3(0.04), params.albedo, params.metallic);
    vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

    float NDF = DistributionGGX(N, H, params.roughness);
    float G = GeometrySmith(N, V, L, params.roughness);

    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - params.metallic;

    vec3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0);
    vec3 specularPart = numerator / max(denominator, 0.0000001);

    float NdotL = max(dot(N, L), 0.0);
    vec3 diffusePart = kD * params.albedo / PI;

    vec3 radiance = light_color * light_intensity;
    vec3 directLight = (diffusePart + specularPart) * radiance * NdotL;

    return directLight;
}

void main()
{
    vec3 N = GetNormal();
    vec3 V = normalize(camera_position - world_position);
    vec3 L = normalize(light_direction);
    vec3 H = normalize(V + L);

    MetallicRoughnessParameters params;
    params.normal = GetNormal();
    params.view_direction = normalize(camera_position - world_position);

    float alpha = 1.0f;

    if (use_spec_gloss != 0)
    {
        vec4 diffuseColorLinear = GetDiffuseColor();
        alpha = diffuseColorLinear.a;

        params.albedo = pow(diffuseColorLinear.rgb, vec3(2.2));
        params.roughness = clamp(1.0 - GetGlossiness(), 0.001, 1.0);
        params.metallic = 0.0;

        //specular = pow(GetSpecularColor(), vec3(2.2));
    }
    else
    {
        vec4 baseColorLinear = GetBaseColor();
        alpha = baseColorLinear.a;

        params.albedo = pow(baseColorLinear.rgb, vec3(2.2));
        params.roughness = clamp(GetRoughness(), 0.001, 1.0);
        params.metallic = clamp(GetMetallic(), 0.0, 1.0);
    }

    vec3 directLight = PBRShade(params);

    float ao = 1.0;
    vec3 ambient = vec3(0.03) * params.albedo * ao;
    vec3 emission = GetEmissionColor().rgb * GetEmissionColor().a;

    vec3 finalColor = ambient + directLight + emission;

    finalColor = finalColor / (finalColor + vec3(1.0));
    finalColor = pow(finalColor, vec3(1.0/2.2));

    FragColor = vec4(finalColor, alpha);
}

)";

static inline constexpr std::string_view Depth_Map_VS = R"(
#version 450 core
layout(location = 0) in vec3 in_position;

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
layout(location = 3) out vec3 tangent;
layout(location = 4) out vec3 bitangent;

void main()
{
    vec4 model_position = model * vec4(in_position, 1.0);
    gl_Position = projection * view * model_position;

    world_position = model_position.xyz;
    normal = transpose(mat3(model)) * in_normal;
    tex_coord = in_texcoord;
    tangent = transpose(mat3(model)) * in_tangent;
    bitangent = transpose(mat3(model)) * in_bitangent;
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
                        {"specular_color", 4 * sizeof(float)},
                        {"emission_color", 4 * sizeof(float)},
                        {"metallic", sizeof(float)},
                        {"roughness", sizeof(float)},
                        {"glossiness", sizeof(float)},
                        {"normal_scale", sizeof(float)},
                        {"use_spec_gloss", sizeof(uint32_t)},
                        {"use_emission_color_map", sizeof(uint32_t)},
                        {"use_base_color_map", sizeof(uint32_t)},
                        {"use_metallic_map", sizeof(uint32_t)},
                        {"use_roughness_map", sizeof(uint32_t)},
                        {"use_specular_map", sizeof(uint32_t)},
                        {"use_glossiness_map", sizeof(uint32_t)},
                        {"use_normal_map", sizeof(uint32_t)}});
    auto binding3 = ShaderBinding{3u, ShaderStage::Fragment, ShaderBinding::Type::UniformBuffer};
    binding3.SetLayout({{"camera_position", 3 * sizeof(float)},
                        {"light_direction", 3 * sizeof(float)},
                        {"light_color", 3 * sizeof(float)},
                        {"light_intensity", sizeof(float)}});

    auto materialTexture = std::make_unique<ImageTexture>();
    materialTexture->SetAddressModeUV(AddressMode::Repeat, AddressMode::Repeat);

    auto binding4 = ShaderBinding{4u, ShaderStage::Fragment, ShaderBinding::Type::SampledTexture};
    binding4.SetTexture(ShaderBindingTexture("base_color_map", materialTexture->Clone()));
    auto binding5 = ShaderBinding{5u, ShaderStage::Fragment, ShaderBinding::Type::SampledTexture};
    binding5.SetTexture(ShaderBindingTexture("emission_color_map", materialTexture->Clone()));
    auto binding6 = ShaderBinding{6u, ShaderStage::Fragment, ShaderBinding::Type::SampledTexture};
    binding6.SetTexture(ShaderBindingTexture("metallic_map", materialTexture->Clone()));
    auto binding7 = ShaderBinding{7u, ShaderStage::Fragment, ShaderBinding::Type::SampledTexture};
    binding7.SetTexture(ShaderBindingTexture("roughness_map", materialTexture->Clone()));
    auto binding8 = ShaderBinding{8u, ShaderStage::Fragment, ShaderBinding::Type::SampledTexture};
    binding8.SetTexture(ShaderBindingTexture("normal_map", materialTexture->Clone()));
    auto binding9 = ShaderBinding{9u, ShaderStage::Fragment, ShaderBinding::Type::SampledTexture};
    binding9.SetTexture(ShaderBindingTexture("specular_map", materialTexture->Clone()));
    auto binding10 = ShaderBinding{10u, ShaderStage::Fragment, ShaderBinding::Type::SampledTexture};
    binding10.SetTexture(ShaderBindingTexture("glossiness_map", std::move(materialTexture)));

    fragShader->AddBinding(binding2);
    fragShader->AddBinding(binding3);
    fragShader->AddBinding(binding4);
    fragShader->AddBinding(binding5);
    fragShader->AddBinding(binding6);
    fragShader->AddBinding(binding7);
    fragShader->AddBinding(binding8);
    fragShader->AddBinding(binding9);
    fragShader->AddBinding(binding10);

    m_fragmentShaders["PBR_FS"] = fragShader;
}

} // namespace CS
