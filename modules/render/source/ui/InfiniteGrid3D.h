#pragma once
#include "math/Matrix.h"
#include "math/Vector.h"
#include <memory>

namespace CS
{

class Scene;
class SceneObject;
class MaterialInstance;

// Blender-style "layout mode" infinite world grid: a full-screen quad is ray-cast
// onto a world-space plane in the vertex shader, then the fragment shader draws
// anti-aliased sub / major grid lines and tinted world-space axes on top.
//
// The viewType is 3D_Main so the grid is rendered into the same target as the
// PBR pass. Depth test is enabled so opaque scene objects correctly occlude
// the grid.
class InfiniteGrid3D
{
public:
    InfiniteGrid3D(std::shared_ptr<Scene> scene);
    ~InfiniteGrid3D();

    // Plane configuration -----------------------------------------------------
    // Origin of the infinite plane in world space (defaults to the world
    // origin).
    void SetPlaneOrigin(const Vector3f& origin);
    // Unit normal of the infinite plane. Defaults to (0, 1, 0) which matches
    // the engine's Y-up convention, placing the grid on the XZ plane. The axis
    // corresponding to the normal is also suppressed by the shader so only the
    // two in-plane axes receive coloured axis lines.
    void SetPlaneNormal(const Vector3f& normal);

    // Grid appearance ---------------------------------------------------------
    // World-space distance between two adjacent sub-grid lines.
    void SetGridSize(float size);
    // How many sub-grid steps fit between two major-grid lines.
    void SetSubGridCount(float count);
    // Half-width (in sub-grid cells) of the coloured world-space axis lines.
    void SetAxisWidth(float width);

    void SetCameraPosition(const Vector3f& position);

    void SetMajorGridColor(const Vector3f& color);
    void SetSubGridColor(const Vector3f& color);
    void SetXAxisColor(const Vector3f& color);
    void SetYAxisColor(const Vector3f& color);
    void SetZAxisColor(const Vector3f& color);

    // Per-frame update: pushes the camera position uniform and recomputes a
    // view-adaptive grid size via CalculateGridSize(). Intended to be called
    // from the owner's OnUpdate hook.
    //
    // The projection matrix is consumed so the calculator can see the camera
    // FOV (perspective) or ortho extent and scale cells accordingly -- the
    // previous altitude-only formula produced cells that were much too fine
    // for wide FOVs / orthographic views.
    void Update(const Vector3f& cameraPosition, const Matrix4f& projectionMatrix);

    // Pure, stateless grid-size LOD calculator.
    //
    // Given the camera-to-plane-origin distance and the active projection
    // matrix, returns a sub-grid step (the world-space distance between two
    // adjacent sub-grid lines) snapped to a power of 10 so the screen-space
    // footprint of a cell stays roughly constant as the camera zooms or the
    // FOV changes.
    //
    // Principle: evaluate the world-space vertical extent the camera sees at
    // the plane, then divide by `targetCellsVertical`.
    //   Perspective: visible world height at `viewingDistance` from the eye
    //               is 2 * viewingDistance * tan(fovy/2)
    //                 = 2 * viewingDistance / P[1][1].
    //   Orthographic: visible world height is (top - bottom)
    //                 = 2 / P[1][1], independent of distance.
    // The perspective vs. ortho mode is detected from P[3][3] (1 for ortho,
    // 0 for perspective).
    //
    // The raw size is then snapped to the nearest power of 10 in log space
    // (transitions at x ~= sqrt(10) ~= 3.16). With sub_grid_count == 10 this
    // makes the major-grid lines of one decade become the sub-grid lines of
    // the next, producing the Blender-style seamless "decade snap" as the
    // camera dollies in or out.
    //
    // `targetCellsVertical` controls density: smaller -> bigger cells.
    static float
    CalculateGridSize(float viewingDistance, const Matrix4f& projectionMatrix, float targetCellsVertical = 10.0f);

private:
    std::shared_ptr<Scene> m_scene;
    std::shared_ptr<SceneObject> m_sceneObject;
    std::shared_ptr<MaterialInstance> m_material;

    // Cached plane parameters used by the grid-size LOD calculator so we do
    // not have to round-trip them through the uniform block.
    Vector3f m_planeOrigin{0.0f, 0.0f, 0.0f};
    Vector3f m_planeNormal{0.0f, 1.0f, 0.0f};
};

} // namespace CS
