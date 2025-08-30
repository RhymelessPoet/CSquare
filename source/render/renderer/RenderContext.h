#pragma once
#include "graphics/GraphicsCommandBuffer.h"
#include <memory>

namespace CS
{
class GraphicsAPI;
class Camera;
class Material;
class MaterialComputer;
class RenderContext
{
public:
    RenderContext(std::shared_ptr<GraphicsAPI> graphicsAPI);
    virtual ~RenderContext() = default;

    GraphicsCommandBuffer GetCommandBuffer() const { return m_commandBuffer; }

    void SetCamera(std::shared_ptr<Camera> camera) { m_camera = std::move(camera); }
    std::shared_ptr<Camera> GetCamera() const { return m_camera; }

    MaterialComputer& GetMaterialComputer() { return *m_materialComputer; }

    std::shared_ptr<GraphicsAPI> GetGraphicsAPI() { return m_graphicsAPI; }

private:
    std::shared_ptr<GraphicsAPI> m_graphicsAPI;
    std::unique_ptr<MaterialComputer> m_materialComputer;
    std::shared_ptr<Camera> m_camera;
    GraphicsCommandBuffer m_commandBuffer;
};

} // namespace CS
