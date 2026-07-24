#pragma once
#include "Size.h"
#include "IModule.h"
#include <functional>
#include <memory>
#include <string>

struct GLFWwindow;

namespace CS
{
class EngineController;
class RenderModule;
class View;
class GraphicsAPI;
class CameraManipulator;
class IRenderSample;
class SceneObjectComposer;

// Standalone GLFW window host that drives an IRenderSample via the engine's
// module lifecycle. Added to Engine AFTER RenderModule so that:
//   * SampleModule::Initialize() can resolve RenderModule via
//     Engine::GetModule<RenderModule>().
//   * Each frame RenderModule::Update() (render-to-FBO) runs before
//     SampleModule::Update() (present + input polling).
class SampleModule final : public IModule
{
public:
    using SampleFactory = std::function<std::unique_ptr<IRenderSample>(std::shared_ptr<SceneObjectComposer>)>;

    SampleModule(std::string title, Size2u initialSize, SampleFactory factory);
    ~SampleModule() override;

    SampleModule(const SampleModule&) = delete;
    SampleModule& operator=(const SampleModule&) = delete;

    void Initialize() override;
    void Update() override;
    void SetEngineController(std::shared_ptr<EngineController> controller) override { m_controller = controller; }

private:
    void installGLFWCallbacks();
    void fitToScene(bool recompute);
    void onFramebufferResize(int width, int height);
    void onMouseButton(int button, int action, int mods);
    void onCursorPos(double xPos, double yPos);
    void onScroll(double xOffset, double yOffset);
    void onKey(int key, int scancode, int action, int mods);
    void updateFPSTitle();

private:
    struct MouseState
    {
        // GLFW mouse-button state (bit = 1 when pressed)
        uint32_t pressedButtons{0u};
        double lastX{0.0};
        double lastY{0.0};
    };

    std::string m_title;
    Size2u m_initialSize;
    SampleFactory m_factory;

    std::shared_ptr<EngineController> m_controller;

    GLFWwindow* m_window{nullptr};
    RenderModule* m_renderModule{nullptr};
    std::shared_ptr<View> m_view;
    std::shared_ptr<GraphicsAPI> m_graphicsAPI;
    std::unique_ptr<CameraManipulator> m_cameraManipulator;
    std::unique_ptr<IRenderSample> m_sample;

    Size2u m_windowSize{1u, 1u};
    MouseState m_mouseState;
    bool m_initialized{false};

    // FPS tracking: accumulate frames between title updates and sample elapsed
    // time via glfwGetTime() to avoid pulling in <chrono> here.
    double m_fpsWindowStart{0.0};
    uint32_t m_fpsFrameCount{0u};
};

} // namespace CS
