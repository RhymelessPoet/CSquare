#pragma once
#include "Rect.h"
#include "graphics/GraphicsCommandBuffer.h"
#include "scene/ViewType.h"
#include <memory>

namespace CS
{
class GraphicsAPI;
class Camera;
class Material;
class MaterialCompiler;

class RenderContext
{
public:
    RenderContext(std::shared_ptr<GraphicsAPI> graphicsAPI);
    virtual ~RenderContext() = default;

    GraphicsCommandBuffer GetCommandBuffer() const { return m_commandBuffer; }
    std::shared_ptr<GraphicsAPI> GetGraphicsAPI() { return m_graphicsAPI; }

    void SetCamera(std::shared_ptr<Camera> camera) { m_camera = std::move(camera); }
    std::shared_ptr<Camera> GetCamera() const { return m_camera; }

    // Observer camera: the camera whose output is ultimately shown to the user (3D_Main view).
    // Exposed to pre-passes (e.g. shadow map) so they can cull/focus on what the user will see.
    void SetObserverCamera(std::shared_ptr<Camera> camera) { m_observerCamera = std::move(camera); }
    std::shared_ptr<Camera> GetObserverCamera() const { return m_observerCamera; }

    void SetMaterialCompiler(std::unique_ptr<MaterialCompiler> compiler);
    MaterialCompiler& GetMaterialCompiler() { return *m_materialCompiler; }

    void SetTargetViewType(EViewType viewType) { m_targetViewType = viewType; }
    EViewType GetTargetViewType() const { return m_targetViewType; }

    void SetViewport(const Rect32i& viewport) { m_viewport = viewport; }
    const Rect32i& GetViewport() const { return m_viewport; }

private:
    std::shared_ptr<GraphicsAPI> m_graphicsAPI;
    std::unique_ptr<MaterialCompiler> m_materialCompiler;
    std::shared_ptr<Camera> m_camera;
    std::shared_ptr<Camera> m_observerCamera;
    GraphicsCommandBuffer m_commandBuffer;

    Rect32i m_viewport{0, 0, 720, 720};

    EViewType m_targetViewType;
};

} // namespace CS
