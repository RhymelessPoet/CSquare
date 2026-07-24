#include "InfiniteGrid3D.h"
#include "asset/BuiltInShaders.h"
#include "Logger.h"
#include "geometry/GeometryNode.h"
#include "graphics/GraphicsPipeline.h"
#include "graphics/VertexInputLayout.h"
#include "materials/IMaterialConfiguration.h"
#include "materials/Material.h"
#include "materials/Shader.h"
#include "scene/MeshRenderSystem.h"
#include "scene/MeshRenderer.h"
#include "scene/Scene.h"
#include "scene/SceneObject.h"
#include "scene/SceneObjectComposer.h"
#include "scene/Transform.h"
#include "scene/TransformSystem.h"

#include <algorithm>
#include <cmath>

namespace CS
{

class InfiniteGrid3DMaterialConfiguration : public IMaterialConfiguration
{
public:
    InfiniteGrid3DMaterialConfiguration()
    {
        m_viewType = EViewType::Make<"3D_Main">();
        m_targetSlots.push_back(SlotDescription{.id = {.slotIndex = 0u}, .type = SlotType::Color});
        m_targetSlots.push_back(SlotDescription{.id = {.slotIndex = 0u}, .type = SlotType::Depth});
    }
    // Depth test is enabled so opaque objects occlude the grid. The vertex
    // shader already projects each quad corner back into clip space at its
    // plane-intersection depth, so the grid fragments carry sensible depth
    // values. The fragment shader emits a non-premultiplied (color, alpha)
    // where alpha encodes both the grid-line coverage and the grazing-angle
    // fade, so we enable standard alpha blending:
    //     finalRGB   = srcRGB * srcAlpha + dstRGB * (1 - srcAlpha)
    //     finalAlpha = srcAlpha * 1      + dstAlpha * (1 - srcAlpha)
    // This way grazing-angle pixels dissolve into the framebuffer contents
    // instead of being a binary discard, and sub-line anti-alias coverage is
    // preserved as a real translucent composite.
    //
    // Depth clamp is enabled because the VS ray-casts each full-screen quad
    // corner onto the infinite plane and then projects the hit back into
    // clip space. Near the horizon the resulting gl_Position.z can exceed
    // the [-1, 1] NDC range and get near/far-plane clipped, leaving a
    // visible seam at the frustum edge. Clamping instead of clipping keeps
    // those horizon fragments alive and lets them fade via their own alpha.
    void Configure(GraphicsPipeline& pipeline) const override
    {
        pipeline.SetDepthTest(true);
        pipeline.SetDepthCompareOP(DepthCompareOp::Less);
        pipeline.SetDepthClamp(true);
        pipeline.SetBlendEnable(true);
        pipeline.SetBlendColorFactors(BlendFactor::SrcAlpha, BlendFactor::OneMinusSrcAlpha);
        pipeline.SetBlendAlphaFactors(BlendFactor::One, BlendFactor::OneMinusSrcAlpha);
        pipeline.SetBlendColorOp(BlendOp::Add);
        pipeline.SetBlendAlphaOp(BlendOp::Add);
    }
};

// Subdivided screen-space grid in NDC. An 8x8 lattice of vertices (64 verts,
// 128 floats total because each vertex is a vec2) spans [-1,1] x [-1,1].
// Subdivision gives interior vertices to the vertex shader so its ray-cast
// world positions are produced at an 8x8 lattice rather than just four
// corners; downstream per-pixel math that relies on interpolated
// world_position (grazing-angle fade, sub-grid masking) then runs with
// finer-grained input data.
static constexpr uint32_t GRID_SUBDIV = 8u;
static constexpr uint32_t GRID_VERTEX_COUNT = GRID_SUBDIV * GRID_SUBDIV;

// clang-format off
static const std::vector<float> vertices = [] {
    std::vector<float> v;
    v.reserve(GRID_VERTEX_COUNT * 2u);
    for (uint32_t j = 0u; j < GRID_SUBDIV; ++j) {
        const float y = -1.0f + 2.0f * static_cast<float>(j) / static_cast<float>(GRID_SUBDIV - 1u);
        for (uint32_t i = 0u; i < GRID_SUBDIV; ++i) {
            const float x = -1.0f + 2.0f * static_cast<float>(i) / static_cast<float>(GRID_SUBDIV - 1u);
            v.push_back(x);
            v.push_back(y);
        }
    }
    return v;
}();

static const std::vector<uint32_t> indices = [] {
    // (N-1)*(N-1) cells, 2 triangles each, 3 indices per triangle.
    std::vector<uint32_t> idx;
    idx.reserve((GRID_SUBDIV - 1u) * (GRID_SUBDIV - 1u) * 6u);
    for (uint32_t j = 0u; j < GRID_SUBDIV - 1u; ++j) {
        for (uint32_t i = 0u; i < GRID_SUBDIV - 1u; ++i) {
            const uint32_t a = j * GRID_SUBDIV + i;            // bottom-left
            const uint32_t b = a + 1u;                          // bottom-right
            const uint32_t c = a + GRID_SUBDIV;                 // top-left
            const uint32_t d = c + 1u;                          // top-right
            // Preserve the original quad's winding {TL, TR, BL, TR, BR, BL}.
            idx.push_back(c); idx.push_back(d); idx.push_back(a);
            idx.push_back(d); idx.push_back(b); idx.push_back(a);
        }
    }
    return idx;
}();
// clang-format on

InfiniteGrid3D::InfiniteGrid3D(std::shared_ptr<Scene> scene) : m_scene(std::move(scene))
{
    m_sceneObject = m_scene->CreateSceneObject();
    m_sceneObject->SetName("InfiniteGrid3D");

    auto composer = m_scene->GetComposer();
    composer->AddComponent<MeshRenderer>(m_sceneObject);
    composer->AddComponent<Transform>(m_sceneObject);

    auto& meshRenderer = GetComponent<MeshRenderer>(m_sceneObject);

    // clang-format off
    auto material = Material::Builder()
                    .Begin(std::make_unique<InfiniteGrid3DMaterialConfiguration>())
                    .AddInputAttribute(0u, VertexInputFormat::Float2)
                    .AddShader(BuiltInShaders::Instance().GetVertexShader("InfiniteGrid3D_VS"))
                    .AddShader(BuiltInShaders::Instance().GetFragmentShader("InfiniteGrid3D_FS"))
                    .End();
    // clang-format on

    // Seed the GridParams uniform block with Blender-like defaults on the
    // default instance before any instance is cloned. Y-up convention: plane
    // normal is +Y so the grid lies on the XZ plane; X gets a red axis, Z a
    // blue axis, and the Y-axis colour is effectively unused because the
    // shader masks it out via (1 - plane_normal).
    (void)material->SetUniformValue("camera_position", Vector3f{0.0f, 10.0f, 10.0f});
    (void)material->SetUniformValue("plane_origin", Vector3f{0.0f, 0.0f, 0.0f});
    (void)material->SetUniformValue("plane_normal", Vector3f{0.0f, 1.0f, 0.0f});
    (void)material->SetUniformValue("major_grid_color", Vector3f{1.0f, 1.0f, 1.0f});
    (void)material->SetUniformValue("sub_grid_color", Vector3f{0.6f, 0.6f, 0.6f});
    (void)material->SetUniformValue("x_axis_color", Vector3f{1.0f, 0.0f, 0.0f});
    (void)material->SetUniformValue("y_axis_color", Vector3f{0.0f, 1.0f, 0.0f});
    (void)material->SetUniformValue("z_axis_color", Vector3f{0.0f, 0.0f, 1.0f});
    (void)material->SetUniformValue("grid_size", 0.01f);
    (void)material->SetUniformValue("sub_grid_count", 10.0f);
    (void)material->SetUniformValue("axis_width", 1.5f);

    m_material = material->CreateInstance();

    auto mesh =
        Mesh::Builder()
            .AddVertexBuffer(vertices)
            .SetVertexCount(GRID_VERTEX_COUNT)
            .AddAttribute(Mesh::Attribute{"_position", 0u, static_cast<uint32_t>(vertices.size() * sizeof(float)), 0u,
                                          2u, DataType::Float32})
            .SetIndices(Buffer(indices), DataType::UInt32)
            .Build();

    auto geometryNode = std::make_shared<GeometryNode>(mesh, m_material);
    geometryNode->SetAttributeMap("_position", 0u);
    meshRenderer.AddGeometryNode(geometryNode);
}

InfiniteGrid3D::~InfiniteGrid3D()
{
    m_sceneObject->GetParent()->RemoveChild(m_sceneObject);
}

void InfiniteGrid3D::SetPlaneOrigin(const Vector3f& origin)
{
    m_planeOrigin = origin;
    if (!m_material->SetUniformValue("plane_origin", origin)) {
        CS::LogError(::CS::BuiltInChannels::Render(), "InfiniteGrid3D: SetUniformValue('plane_origin') failed");
    }
}

void InfiniteGrid3D::SetPlaneNormal(const Vector3f& normal)
{
    m_planeNormal = normal;
    if (!m_material->SetUniformValue("plane_normal", normal)) {
        CS::LogError(::CS::BuiltInChannels::Render(), "InfiniteGrid3D: SetUniformValue('plane_normal') failed");
    }
}

void InfiniteGrid3D::SetGridSize(float size)
{
    if (!m_material->SetUniformValue("grid_size", size)) {
        CS::LogError(::CS::BuiltInChannels::Render(), "InfiniteGrid3D: SetUniformValue('grid_size') failed");
    }
}

void InfiniteGrid3D::SetSubGridCount(float count)
{
    if (!m_material->SetUniformValue("sub_grid_count", count)) {
        CS::LogError(::CS::BuiltInChannels::Render(), "InfiniteGrid3D: SetUniformValue('sub_grid_count') failed");
    }
}

void InfiniteGrid3D::SetAxisWidth(float width)
{
    if (!m_material->SetUniformValue("axis_width", width)) {
        CS::LogError(::CS::BuiltInChannels::Render(), "InfiniteGrid3D: SetUniformValue('axis_width') failed");
    }
}

void InfiniteGrid3D::SetCameraPosition(const Vector3f& position)
{
    if (!m_material->SetUniformValue("camera_position", position)) {
        CS::LogError(::CS::BuiltInChannels::Render(), "InfiniteGrid3D: SetUniformValue('camera_position') failed");
    }
}

void InfiniteGrid3D::SetMajorGridColor(const Vector3f& color)
{
    (void)m_material->SetUniformValue("major_grid_color", color);
}

void InfiniteGrid3D::SetSubGridColor(const Vector3f& color)
{
    (void)m_material->SetUniformValue("sub_grid_color", color);
}

void InfiniteGrid3D::SetXAxisColor(const Vector3f& color)
{
    (void)m_material->SetUniformValue("x_axis_color", color);
}

void InfiniteGrid3D::SetYAxisColor(const Vector3f& color)
{
    (void)m_material->SetUniformValue("y_axis_color", color);
}

void InfiniteGrid3D::SetZAxisColor(const Vector3f& color)
{
    (void)m_material->SetUniformValue("z_axis_color", color);
}

void InfiniteGrid3D::Update(const Vector3f& cameraPosition, const Matrix4f& projectionMatrix)
{
    SetCameraPosition(cameraPosition);

    // Scale the LOD off the full camera-to-plane-origin distance (orbit
    // radius for orbit cameras) rather than the perpendicular altitude.
    // Altitude collapses to ~0 as the camera tilts to grazing and would
    // force an absurdly fine grid near the horizon; distance-to-origin stays
    // well behaved there.
    const float distance = (cameraPosition - m_planeOrigin).Length();
    SetGridSize(CalculateGridSize(distance, projectionMatrix));
}

float InfiniteGrid3D::CalculateGridSize(float viewingDistance,
                                        const Matrix4f& projectionMatrix,
                                        float targetCellsVertical)
{
    // Row-major accessor: projectionMatrix[row][col]. The entries we need
    // are the same in either row-major or column-major storage because the
    // projection matrix is diagonal in these positions.
    const float P11 = projectionMatrix[1][1];
    const float P33 = projectionMatrix[3][3];

    // Orthographic projection has P[3][3] = 1 (row 3 = (0,0,0,1)); a standard
    // perspective matrix has P[3][3] = 0 (row 3 carries the -w term).
    const bool isOrtho = std::abs(P33 - 1.0f) < 0.5f;

    // Guard against degenerate matrices: a zero P11 would send worldHeight
    // to infinity and explode the log.
    const float safeP11 = std::max(std::abs(P11), 1e-6f);

    // World-space vertical extent visible at the plane.
    //   Perspective: 2 * D * tan(fovy/2) = 2 * D / P11.
    //   Orthographic: (top - bottom) = 2 / P11 (constant in distance).
    const float worldHeight = isOrtho ? (2.0f / safeP11) : (2.0f * std::max(viewingDistance, 0.0f) / safeP11);

    const float cells = std::max(targetCellsVertical, 1.0f);
    const float rawGridSize = std::max(worldHeight / cells, 1e-6f);

    // Snap to the nearest power of 10 *in log space* (round, not floor), so
    // the decade transition happens around rawGridSize ~= sqrt(10) ~= 3.16
    // instead of at each power-of-10 boundary. This gives roughly symmetric
    // up/down rounding and keeps cells one decade coarser than the previous
    // floor-based formula where the user complained they were too fine.
    const float exponent = std::round(std::log10(rawGridSize));
    return std::pow(10.0f, exponent);
}

} // namespace CS
