#include "SInfiniteGrid3D.h"

#include "Color.h"
#include "scene/Camera.h"
#include "scene/Scene.h"
#include "scene/View.h"
#include "ui/InfiniteGrid3D.h"

namespace CS
{
SInfiniteGrid3D::SInfiniteGrid3D(std::shared_ptr<SceneObjectComposer> composer) : IRenderSample(std::move(composer)) {}

SInfiniteGrid3D::~SInfiniteGrid3D() = default;

void SInfiniteGrid3D::Initialize(std::shared_ptr<View> view)
{
    m_scene = std::make_shared<CS::Scene>(m_composer);

    m_grid = std::make_unique<InfiniteGrid3D>(m_scene);

    view->SetScene(m_scene);

    // Solid mid-grey background so the white major-grid lines and coloured
    // axis lines are both clearly visible.
    view->SetClearColor(Color(0.4f, 0.4f, 0.4f, 1.0f));

    m_view = std::move(view);
}

// Per-frame: feed the current camera position and projection matrix to the
// grid so (a) the fragment shader can evaluate the per-pixel grazing fade
// and (b) the grid-size LOD calculator can pick a decade that keeps
// sub-cells at a usable on-screen size as the orbit camera dollies toward
// / away from the plane or the FOV / projection mode changes.
void SInfiniteGrid3D::OnUpdate()
{
    if (!m_grid || !m_view) {
        return;
    }
    auto camera = m_view->GetCamera();
    if (!camera) {
        return;
    }
    m_grid->Update(camera->GetPosition(), camera->GetProjectionMatrix());
}

} // namespace CS
