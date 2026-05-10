#include "BuiltInShaders.h"
#include "graphics/RenderTexture.h"
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

// 光源参数
layout(std140, binding = 3) uniform ImagingParameters
{
    mat4 light_vp_matrix;
    vec3 camera_position;
    vec3 light_direction;
    vec3 light_color;
    vec2 shadow_normal_bias;
    float light_intensity;
};

layout(location = 0) out vec3 normal;
layout(location = 1) out vec3 world_position;
layout(location = 2) out vec2 tex_coord;
layout(location = 3) out vec3 tangent;
layout(location = 4) out vec3 bitangent;
layout(location = 5) out vec4 light_space_position;

highp vec4 GetLightSpacePosition(highp vec3 p, const highp vec3 n, const highp vec2 b) {

    mat4 clipMat = mat4(vec4(0.5, 0.0, 0.0, 0.0), 
                        vec4(0.0, 0.5, 0.0, 0.0), 
                        vec4(0.0, 0.0, 0.5, 0.0), 
                        vec4(0.5, 0.5, 0.5, 1.0));
    highp mat4 lightFromWorldMatrix = clipMat * light_vp_matrix;

    // Extract the first row (Light's Right vector in World Space)
    highp vec3 L_right = vec3(lightFromWorldMatrix[0][0], lightFromWorldMatrix[1][0], lightFromWorldMatrix[2][0]);

    // Extract the second row (Light's Up vector in World Space)
    highp vec3 L_up    = vec3(lightFromWorldMatrix[0][1], lightFromWorldMatrix[1][1], lightFromWorldMatrix[2][1]);

    // Project the world normal onto the shadow map's 2D grid
    highp float n_Lx = dot(n, L_right);
    highp float n_Ly = dot(n, L_up);

    // Apply the anisotropic normal bias (Castaño 2013)
    p += n * (abs(n_Lx * b.x) + abs(n_Ly * b.y));

    vec4 lsPos = lightFromWorldMatrix * vec4(p, 1.0f);
    return lsPos;
}


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

    light_space_position = GetLightSpacePosition(world_position, normalize(normal), shadow_normal_bias);
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
layout(location = 5) in vec4 light_space_position;


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

    unsigned int receive_shadow;
};

// 光源参数
layout(std140, binding = 3) uniform ImagingParameters
{
    mat4 light_vp_matrix;
    vec3 camera_position;
    vec3 light_direction;
    vec3 light_color;
    vec2 shadow_normal_bias;
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

layout(binding = 11) uniform sampler2D shadow_map;

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
    vec3 L = normalize(-light_direction);
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

float SampleShadow(highp vec2 uv, float depth)
{
    // Clamp to shadow map bounds (cf. Filament's scissorNormalized clamp)
    uv = clamp(uv, vec2(0.0), vec2(1.0));
    return texture(shadow_map, uv).r < depth ? 1.0 : 0.0;
}

float CastanoBilinearPCF(highp vec2 pos, float depth, highp vec2 size, highp vec2 texelSize)
{
    //  Castaño, 2013, "Shadow Mapping Summary Part 1"
    //  Same algorithm as Filament's ShadowSample_PCF_Low
    vec2 offset = vec2(0.5);
    highp vec2 uv = (pos * size) + offset;
    highp vec2 base = (floor(uv) - offset) * texelSize;
    highp vec2 st = fract(uv);

    vec2 uw = vec2(3.0 - 2.0 * st.x, 1.0 + 2.0 * st.x);
    vec2 vw = vec2(3.0 - 2.0 * st.y, 1.0 + 2.0 * st.y);

    highp vec2 u = vec2((2.0 - st.x) / uw.x - 1.0, st.x / uw.y + 1.0);
    highp vec2 v = vec2((2.0 - st.y) / vw.x - 1.0, st.y / vw.y + 1.0);

    u *= texelSize.x;
    v *= texelSize.y;

    float sum = 0.0;
    sum += uw.x * vw.x * SampleShadow(base + vec2(u.x, v.x), depth);
    sum += uw.y * vw.x * SampleShadow(base + vec2(u.y, v.x), depth);
    sum += uw.x * vw.y * SampleShadow(base + vec2(u.x, v.y), depth);
    sum += uw.y * vw.y * SampleShadow(base + vec2(u.y, v.y), depth);
    return sum * (1.0 / 16.0);
}

float GetPCFShadow(vec3 geoNormal, vec3 lightDirection)
{
    highp vec3 position = light_space_position.xyz * (1.0 / light_space_position.w);
    highp vec2 size = vec2(textureSize(shadow_map, 0));
    highp vec2 texelSize = vec2(1.0) / size;

    float bias = max(0.05 * (1.0 - dot(geoNormal, lightDirection)), 0.005);
    highp float depth = position.z - bias;

    // clamp position to avoid overflows below, which cause some GPUs to abort
    position.xy = clamp(position.xy, vec2(-1.0), vec2(2.0));

    // Multi-tap Castaño PCF for soft shadows.
    // Filament's ShadowSample_PCF_Low uses sampler2DArrayShadow which gives
    // 4 effective samples per texture fetch (hardware depth comparison + bilinear).
    // With sampler2D + Nearest filtering, each fetch gives 1 effective sample,
    // so we use multiple Castaño taps to cover a wider area for soft penumbra.
    float shadow = 0.0;
    const float spread = 2.0;

    // 2x2 offset grid: 4 taps x 4 Castaño samples = 16 effective PCF samples
    shadow += CastanoBilinearPCF(position.xy + vec2(-0.5, -0.5) * texelSize * spread, depth, size, texelSize);
    shadow += CastanoBilinearPCF(position.xy + vec2( 0.5, -0.5) * texelSize * spread, depth, size, texelSize);
    shadow += CastanoBilinearPCF(position.xy + vec2(-0.5,  0.5) * texelSize * spread, depth, size, texelSize);
    shadow += CastanoBilinearPCF(position.xy + vec2( 0.5,  0.5) * texelSize * spread, depth, size, texelSize);

    return shadow * 0.25;
}

void main()
{
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

    vec3 directLight = PBRShade(params) * (1.0f - GetPCFShadow(normal, -light_direction));

    float ao = 1.0;
    vec3 ambient = vec3(0.05) * params.albedo * ao;
    vec3 emission = GetEmissionColor().rgb * GetEmissionColor().a;

    vec3 finalColor = ambient + directLight;// + emission;

    finalColor = finalColor / (finalColor + vec3(1.0));
    finalColor = pow(finalColor, vec3(1.0/2.2));

    FragColor = vec4(finalColor, alpha);
}

)";

static inline constexpr std::string_view Infinite_Grid_VS = R"(
#version 450 core
layout(location = 0) in vec2 position;
layout(location = 0) out highp vec3 world_position;
// |cos(angle(ray.direction, plane_normal))| evaluated from the un-projected
// per-vertex ray BEFORE the world_position clamp. This is the physically
// correct grazing cosine: it goes to 0 exactly where the ray is parallel
// to the plane (the horizon), regardless of whether the intersection
// parameter t blows up. Computing it per-pixel from world_position is
// unsafe because adjacent vertices that straddle the horizon clamp to
// +/-1e30 with opposite signs, so the linearly-interpolated world_position
// near the horizon row collapses toward 0 and yields a bogus "looking
// straight down" cosine, leaving a bright band of unfaded grid at the
// horizon. Interpolating this pre-clamp cosine across the 8x8 quad keeps
// the horizon band well-defined and linearly ramps through 0 there.
layout(location = 1) out highp float ray_cos_normal;

layout(std140, binding = 0) uniform VPMatrix
{
    mat4 view;
    mat4 projection;
};

layout(std140, binding = 1) uniform GridParams
{
    vec3 camera_position;
    vec3 plane_origin;
    vec3 plane_normal;
    vec3 major_grid_color;
    vec3 sub_grid_color;
    vec3 x_axis_color;
    vec3 y_axis_color;
    vec3 z_axis_color;
    float grid_size;
    float sub_grid_count;
    float axis_width;
};

struct Ray
{
    vec3 origin;
    vec3 direction;
};

Ray GetRay(mat4 viewMatrix, mat4 projMatrix, vec2 ndc)
{
    // Un-project the NDC corner at both the near (z=-1) and far (z=+1) planes
    // back to world space. The perspective divide by w is MANDATORY here:
    // under a perspective projection, inverse(proj)*(x,y,-1,1) and
    // inverse(proj)*(x,y,+1,1) share identical xyz components and differ only
    // in their homogeneous w (1/near vs 1/far). Skipping the divide collapses
    // the direction to a constant (independent of screen x,y), which degenerates
    // the ray-plane intersection and produces a missing/distorted grid.
    vec4 clipNear = vec4(ndc, -1.0, 1.0);
    vec4 clipFar  = vec4(ndc,  1.0, 1.0);

    mat4 invViewProj = inverse(projMatrix * viewMatrix);
    vec4 worldNearH = invViewProj * clipNear;
    vec4 worldFarH  = invViewProj * clipFar;

    vec3 worldNear = worldNearH.xyz / worldNearH.w;
    vec3 worldFar  = worldFarH.xyz  / worldFarH.w;

    Ray ray;
    ray.origin    = worldNear;
    ray.direction = normalize(worldFar - worldNear);
    return ray;
}

vec3 GetIntersectionPoint(mat4 viewMatrix, mat4 projMatrix, vec2 ndc, out float rayCosNormal)
{
    Ray ray = GetRay(viewMatrix, projMatrix, ndc);
    // Record the grazing cosine BEFORE any ray-plane intersection or clamp.
    rayCosNormal = abs(dot(ray.direction, plane_normal));
    ray.origin -= plane_origin;
    float t = dot(-ray.origin, plane_normal) / dot(ray.direction, plane_normal);
    return ray.origin + t * ray.direction;
}

void main()
{
    world_position = GetIntersectionPoint(view, projection, position, ray_cos_normal);
    world_position.x = clamp(world_position.x, -1e30, 1e30);
    world_position.y = clamp(world_position.y, -1e30, 1e30);
    world_position.z = clamp(world_position.z, -1e30, 1e30);

    gl_Position = projection * view * vec4(world_position + plane_origin, 1.0);
}

)";

static inline constexpr std::string_view Infinite_Grid_FS = R"(
#version 450 core

layout(location = 0) in highp vec3 world_position;
// Per-pixel grazing cosine forwarded from the VS. See the VS declaration
// for why this is computed from the pre-clamp ray direction instead of
// being reconstructed here from (world_position - camera_position).
layout(location = 1) in highp float ray_cos_normal;
layout(location = 0) out vec4 FragColor;

layout(std140, binding = 0) uniform VPMatrix
{
    mat4 view;
    mat4 projection;
};

layout(std140, binding = 1) uniform GridParams
{
    vec3 camera_position;
    vec3 plane_origin;
    vec3 plane_normal;
    vec3 major_grid_color;
    vec3 sub_grid_color;
    vec3 x_axis_color;
    vec3 y_axis_color;
    vec3 z_axis_color;
    float grid_size;
    float sub_grid_count;
    float axis_width;
};

vec3 GetAlpha(highp vec3 position, vec3 delta, vec3 width)
{
    vec3 exDistance = 0.5 * (width - 1.0);
    return 1.0 - clamp(position / delta - exDistance, vec3(0.0), vec3(1.0));
}

void main()
{
    float dxx = dFdx(world_position.x);
    float dxy = dFdy(world_position.x);
    float dx  = length(vec2(dxx, dxy));

    float dyx = dFdx(world_position.y);
    float dyy = dFdy(world_position.y);
    float dy  = length(vec2(dyx, dyy));

    float dzx = dFdx(world_position.z);
    float dzy = dFdy(world_position.z);
    float dz  = length(vec2(dzx, dzy));

    vec3 delta = vec3(dx, dy, dz);
    vec3 axes  = 1.0 - plane_normal;

    vec3 disToPlane = step(abs(world_position), axis_width * delta);
    disToPlane = disToPlane * axes;

    vec3 axisColor = disToPlane.yzx + disToPlane.zxy;
    vec3 width     = 1.0 + (axis_width - 1.0) * disToPlane;

    vec3 quotient    = abs(world_position / grid_size);
    vec3 integer     = trunc(quotient);
    vec3 decimal     = quotient - integer;
    vec3 greaterHalf = step(vec3(0.5), decimal);
    vec3 gridNum     = integer + greaterHalf;
    vec3 distance    = abs(greaterHalf - decimal) * grid_size;

    vec3 alphas = GetAlpha(distance, delta, width);
    alphas = alphas * axes;
    vec3 mask = step(alphas.yzx, alphas) * step(alphas.zxy, alphas);
    mask = (mask.yzx + mask.zxy) * axes;

    vec3 isMajorGrids = step(mod(gridNum, vec3(sub_grid_count)), vec3(0.5))
                      * step(vec3(4e-3), alphas);
    float isMajorGrid = max(isMajorGrids.x, max(isMajorGrids.y, isMajorGrids.z));
    vec3 gridColor    = mix(sub_grid_color, major_grid_color, isMajorGrid);

    vec3 xColor = x_axis_color * axisColor.x + (1.0 - axisColor.x) * gridColor;
    vec3 yColor = y_axis_color * axisColor.y + (1.0 - axisColor.y) * gridColor;
    vec3 zColor = z_axis_color * axisColor.z + (1.0 - axisColor.z) * gridColor;

    vec3  color = mask.x * xColor + mask.y * yColor + mask.z * zColor;
    float alpha = max(max(alphas.x, alphas.y), alphas.z);

    // Grazing-angle fade.
    // Use the grazing cosine computed per-vertex from the real un-projected
    // ray direction (see VS). Reconstructing it here from
    // `normalize(world_position - camera_position)` leaves a visible bright
    // band at the horizon because near-horizon pixels interpolate across a
    // pair of vertices whose world_position was clamped to +/-1e30 with
    // opposite signs, so the midpoint collapses to ~0 and fakes a "looking
    // straight down" direction even though the fragment is visually at the
    // horizon.
    //
    // smoothstep(kFadeEnd, kFadeStart, cosTheta) spreads the fade over an
    // explicit ~20-degree window with a C^1-continuous hermite curve, so
    // fragments closer to perpendicular than kFadeStart stay fully opaque,
    // fragments closer to grazing than kFadeEnd are fully transparent, and
    // the middle ramps smoothly (no visible rim at the horizon).
    // Tuning knobs:
    //   kFadeStart - increase to keep more of the grid opaque (smaller fade)
    //   kFadeEnd   - decrease to push the fully-faded rim closer to horizon
    float cosTheta = clamp(ray_cos_normal, 0.0, 1.0);
    const float kFadeEnd   = 0.05;
    const float kFadeStart = 0.35;
    float cosFade  = smoothstep(kFadeEnd, kFadeStart, cosTheta);
    alpha *= cosFade;

    if (alpha > 4e-3) {
        FragColor = vec4(color, alpha);
    } else {
        discard;
    }
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

void main()
{
    vec4 model_position = model * vec4(in_position, 1.0);
    gl_Position = projection * view * model_position;
}

)";

static inline constexpr std::string_view Depth_Map_FS = R"(
#version 450 core

void main() {}

)";

namespace CS
{
BuiltInShaders::BuiltInShaders()
{
    createPanoramicSkyShader();
    createPBRShader();
    createShadowMapShader();
    createInfiniteGrid3DShader();
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
                        {"use_normal_map", sizeof(uint32_t)},
                        {"receive_shadow", sizeof(uint32_t)}});
    auto binding3 = ShaderBinding{3u, ShaderStage::Fragment, ShaderBinding::Type::UniformBuffer};
    binding3.SetLayout({{"light_vp_matrix", 16 * sizeof(float)},
                        {"camera_position", 3 * sizeof(float)},
                        {"light_direction", 3 * sizeof(float)},
                        {"light_color", 3 * sizeof(float)},
                        {"shadow_normal_bias", 2 * sizeof(float)},
                        {"light_intensity", sizeof(float)}});

    auto materialTexture = std::make_unique<ImageTexture>();
    materialTexture->SetAddressModeUV(AddressMode::Repeat, AddressMode::Repeat);

    auto binding4 = ShaderBinding{4u, ShaderStage::Fragment, ShaderBinding::Type::SampledTexture};
    binding4.SetTexture(ShaderBindingTexture("base_color_map", materialTexture->Clone()));
    binding4.GetTexture().texture->SetUseMipmaps(true);
    binding4.GetTexture().texture->SetMipmapFilter(MipmapFilterMode::Linear);
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
    auto binding11 = ShaderBinding{11u, ShaderStage::Fragment, ShaderBinding::Type::SampledTexture};
    // Nearest filtering is required: Castaño PCF does manual depth comparison per texel.
    // Linear filtering would bilinearly interpolate depth values before comparison,
    // causing double-filtering artifacts at shadow edges.
    // ClampToEdge prevents shadow wrapping at shadow map boundaries.
    binding11.SetTexture(ShaderBindingTexture(
        "shadow_map", std::make_unique<RenderTexture>(AddressMode::ClampToEdge, AddressMode::ClampToEdge,
                                                      FilterMode::Nearest, FilterMode::Nearest)));

    fragShader->AddBinding(binding2);
    fragShader->AddBinding(binding3);
    fragShader->AddBinding(binding4);
    fragShader->AddBinding(binding5);
    fragShader->AddBinding(binding6);
    fragShader->AddBinding(binding7);
    fragShader->AddBinding(binding8);
    fragShader->AddBinding(binding9);
    fragShader->AddBinding(binding10);
    fragShader->AddBinding(binding11);

    m_fragmentShaders["PBR_FS"] = fragShader;
}

void BuiltInShaders::createShadowMapShader()
{
    auto vertShader = std::make_shared<Shader>(std::string(Depth_Map_VS), ShaderStage::Vertex);
    auto binding0 = ShaderBinding{0u, ShaderStage::Vertex, ShaderBinding::Type::UniformBuffer};
    binding0.SetLayout({{"view", 16 * sizeof(float)}, {"projection", 16 * sizeof(float)}});
    auto binding1 = ShaderBinding{1u, ShaderStage::Vertex, ShaderBinding::Type::UniformBuffer};
    binding1.SetLayout({{"model", 16 * sizeof(float)}});
    vertShader->AddBinding(binding0);
    vertShader->AddBinding(binding1);
    m_vertexShaders["Depth_Map_VS"] = vertShader;

    auto fragShader = std::make_shared<Shader>(std::string(Depth_Map_FS), ShaderStage::Fragment);
    m_fragmentShaders["Depth_Map_FS"] = fragShader;
}

void BuiltInShaders::createInfiniteGrid3DShader()
{
    // Layout of the GridParams uniform block (std140). Sizes are in bytes.
    // vec3 entries carry size 12 so that ShaderBinding::SetLayout aligns them on
    // 16-byte boundaries, which matches GLSL std140 padding for vec3 members.
    const std::vector<ShaderBindingProperty> gridParamsLayout = {
        {"camera_position", 3 * sizeof(float)}, {"plane_origin", 3 * sizeof(float)},
        {"plane_normal", 3 * sizeof(float)},    {"major_grid_color", 3 * sizeof(float)},
        {"sub_grid_color", 3 * sizeof(float)},  {"x_axis_color", 3 * sizeof(float)},
        {"y_axis_color", 3 * sizeof(float)},    {"z_axis_color", 3 * sizeof(float)},
        {"grid_size", sizeof(float)},           {"sub_grid_count", sizeof(float)},
        {"axis_width", sizeof(float)},
    };

    auto vertShader = std::make_shared<Shader>(std::string(Infinite_Grid_VS), ShaderStage::Vertex);
    auto vsBinding0 = ShaderBinding{0u, ShaderStage::Vertex, ShaderBinding::Type::UniformBuffer};
    vsBinding0.SetLayout({{"view", 16 * sizeof(float)}, {"projection", 16 * sizeof(float)}});
    auto vsBinding1 = ShaderBinding{1u, ShaderStage::Vertex, ShaderBinding::Type::UniformBuffer};
    vsBinding1.SetLayout(gridParamsLayout);
    vertShader->AddBinding(vsBinding0);
    vertShader->AddBinding(vsBinding1);
    m_vertexShaders["InfiniteGrid3D_VS"] = vertShader;

    auto fragShader = std::make_shared<Shader>(std::string(Infinite_Grid_FS), ShaderStage::Fragment);
    auto fsBinding0 = ShaderBinding{0u, ShaderStage::Fragment, ShaderBinding::Type::UniformBuffer};
    fsBinding0.SetLayout({{"view", 16 * sizeof(float)}, {"projection", 16 * sizeof(float)}});
    auto fsBinding1 = ShaderBinding{1u, ShaderStage::Fragment, ShaderBinding::Type::UniformBuffer};
    fsBinding1.SetLayout(gridParamsLayout);
    fragShader->AddBinding(fsBinding0);
    fragShader->AddBinding(fsBinding1);
    m_fragmentShaders["InfiniteGrid3D_FS"] = fragShader;
}

} // namespace CS
