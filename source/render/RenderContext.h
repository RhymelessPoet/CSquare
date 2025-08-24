#pragma once
#include "GraphicsCommandBuffer.h"
#include <memory>

namespace CS
{
class GraphicsAPI;
class Camera;
class RenderContext
{
public:
    RenderContext(std::shared_ptr<GraphicsAPI> graphicsAPI);
    virtual ~RenderContext() = default;

    GraphicsCommandBuffer GetCommandBuffer() const { return m_commandBuffer; }

    void SetCamera(std::shared_ptr<Camera> camera) { m_camera = std::move(camera); }
    std::shared_ptr<Camera> GetCamera() const { return m_camera; }

private:
    std::shared_ptr<GraphicsAPI> m_graphicsAPI;
    std::shared_ptr<Camera> m_camera;
    GraphicsCommandBuffer m_commandBuffer;
};

} // namespace CS
