#include "SampleModule.h"

#include "IRenderSample.h"
#include "Logger.h"
#include "RenderModule.h"
#include "engine/Engine.h"
#include "engine/EngineController.h"
#include "graphics/GLRendererBuilder.h"
#include "graphics/GraphicsAPI.h"
#include "scene/CameraSystem.h"
#include "scene/LightSystem.h"
#include "scene/MeshRenderSystem.h"
#include "scene/Scene.h"
#include "scene/SceneObjectComposer.h"
#include "scene/SystemGraph.h"
#include "scene/TransformSystem.h"
#include "scene/View.h"
#include "utils/AxisAlignedBoundingBox.h"
#include "utils/CameraManipulator.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include <algorithm>
#include <cstdio>
#include <utility>

namespace CS
{
namespace
{
constexpr uint32_t kLeftButtonBit = 1u << GLFW_MOUSE_BUTTON_LEFT;
constexpr uint32_t kRightButtonBit = 1u << GLFW_MOUSE_BUTTON_RIGHT;
constexpr uint32_t kMiddleButtonBit = 1u << GLFW_MOUSE_BUTTON_MIDDLE;

SampleModule* FromWindow(GLFWwindow* window)
{
    return static_cast<SampleModule*>(glfwGetWindowUserPointer(window));
}
} // namespace

SampleModule::SampleModule(std::string title, Size2u initialSize, SampleFactory factory)
    : m_title(std::move(title)), m_initialSize(initialSize), m_factory(std::move(factory))
{}

SampleModule::~SampleModule()
{
    // Release the sample first so that any scene references it owns go away
    // before the renderer (owned by RenderModule) is destroyed later in
    // Engine's dtor. The GLFW window stays alive here so the engine's
    // WGLWindowContext (destroyed when RenderModule tears down) still has a
    // valid HWND at release time; destroying the window only after that
    // would require reordering -- acceptable: main() holds the engine and
    // both modules are destructed within the same scope.
    m_sample.reset();
    m_cameraManipulator.reset();
    m_graphicsAPI.reset();
    m_view.reset();

    if (m_window != nullptr) {
        glfwDestroyWindow(m_window);
        m_window = nullptr;
        glfwTerminate();
    }
}

void SampleModule::Initialize()
{
    if (m_initialized) {
        return;
    }

    if (glfwInit() == GLFW_FALSE) {
        LogError(BuiltInChannels::Render(), "SampleModule: glfwInit failed");
        if (m_controller) {
            m_controller->exit();
        }
        return;
    }

    // Engine owns the sole HGLRC via WGLWindowContext. Create a GLFW window
    // without any GL context so pixel format is not locked; WGLWindowContext
    // will choose one via ChoosePixelFormat/SetPixelFormat itself.
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    m_window = glfwCreateWindow(static_cast<int>(m_initialSize.width), static_cast<int>(m_initialSize.height),
                                m_title.c_str(), nullptr, nullptr);
    if (m_window == nullptr) {
        LogError(BuiltInChannels::Render(), "SampleModule: glfwCreateWindow failed");
        glfwTerminate();
        if (m_controller) {
            m_controller->exit();
        }
        return;
    }

    // Query the actual framebuffer size (may differ from window size on
    // high-DPI displays).
    int fbWidth = 0;
    int fbHeight = 0;
    glfwGetFramebufferSize(m_window, &fbWidth, &fbHeight);
    m_windowSize = Size2u{static_cast<uint32_t>(std::max(fbWidth, 1)), static_cast<uint32_t>(std::max(fbHeight, 1))};

    auto renderModuleOpt = Engine::Instance().GetModule<RenderModule>();
    if (!renderModuleOpt.has_value()) {
        LogError(BuiltInChannels::Render(), "SampleModule: RenderModule is not registered on the engine");
        glfwDestroyWindow(m_window);
        m_window = nullptr;
        glfwTerminate();
        if (m_controller) {
            m_controller->exit();
        }
        return;
    }
    m_renderModule = renderModuleOpt.value();

    // Systems must be registered before the renderer starts updating them;
    // same set as QuickRenderView.
    auto& systemGraph = m_renderModule->GetSystemGraph();
    systemGraph.AddSystem<MeshRenderSystem>();
    systemGraph.AddSystem<LightSystem>();
    systemGraph.AddSystem<CameraSystem>();
    systemGraph.AddSystem<TransformSystem>();

    // Create the renderer bound to the GLFW HWND. No shared HGLRC needed;
    // WGLWindowContext picks its own pixel format.
    HWND hwnd = glfwGetWin32Window(m_window);
    GLRendererBuilder builder;
    builder.SetNativeWindow(reinterpret_cast<void*>(hwnd), nullptr);
    m_renderModule->CreateRenderer(builder);

    m_view = m_renderModule->GetMainView();
    m_graphicsAPI = m_renderModule->GetGraphicsAPI(m_view);

    m_graphicsAPI->MakeContextCurrent();
    m_graphicsAPI->Initialize();

    auto color = m_graphicsAPI->CreateTexture(TextureFormat::RGBA8Unorm, m_windowSize);
    auto depth = m_graphicsAPI->CreateTexture(TextureFormat::Depth32, m_windowSize);

    auto renderTarget = m_graphicsAPI->CreateRenderTarget(m_windowSize);
    renderTarget.SetColorAttachment(color);
    renderTarget.SetDepthAttachment(depth);
    renderTarget.Build();
    m_view->SetRenderTarget(renderTarget);

    m_graphicsAPI->DoneContextCurrent();

    // Build and wire the sample. The factory is supplied by each sample's
    // main() so this module stays sample-agnostic.
    m_sample = m_factory(m_renderModule->GetSOComposer());
    if (m_sample) {
        m_sample->Initialize(m_view);
    }

    m_cameraManipulator = std::make_unique<CameraManipulator>(m_view->GetCamera(), m_windowSize);
    fitToScene(true);

    installGLFWCallbacks();
    m_fpsWindowStart = glfwGetTime();
    m_fpsFrameCount = 0u;
    m_initialized = true;
}

void SampleModule::Update()
{
    if (!m_initialized || m_window == nullptr) {
        return;
    }

    if (glfwWindowShouldClose(m_window) != 0) {
        if (m_controller) {
            m_controller->exit();
        }
        return;
    }

    glfwPollEvents();

    if (m_sample) {
        m_sample->OnUpdate();
    }

    if (m_graphicsAPI && m_view) {
        // RenderModule::Update ran just before us this frame and rendered
        // into the view's FBO. Blit it to the default framebuffer and swap.
        m_graphicsAPI->MakeContextCurrent();
        m_graphicsAPI->Present(m_view->GetRenderTarget(), m_windowSize);
        m_graphicsAPI->DoneContextCurrent();
    }

    updateFPSTitle();
}

void SampleModule::updateFPSTitle()
{
    if (m_window == nullptr) {
        return;
    }

    ++m_fpsFrameCount;

    const double now = glfwGetTime();
    const double elapsed = now - m_fpsWindowStart;
    // Refresh roughly twice a second; short enough to feel live, long enough
    // that GLFW's SetWindowTitle overhead stays negligible.
    constexpr double kRefreshInterval = 0.5;
    if (elapsed < kRefreshInterval) {
        return;
    }

    const double fps = static_cast<double>(m_fpsFrameCount) / elapsed;
    const double msPerFrame = (m_fpsFrameCount > 0u) ? (elapsed * 1000.0 / static_cast<double>(m_fpsFrameCount)) : 0.0;

    char buf[256];
    std::snprintf(buf, sizeof(buf), "%s  |  %.1f FPS  (%.2f ms)", m_title.c_str(), fps, msPerFrame);
    glfwSetWindowTitle(m_window, buf);

    m_fpsWindowStart = now;
    m_fpsFrameCount = 0u;
}

void SampleModule::installGLFWCallbacks()
{
    glfwSetWindowUserPointer(m_window, this);

    glfwSetFramebufferSizeCallback(m_window, [](GLFWwindow* w, int width, int height) {
        if (auto* self = FromWindow(w)) {
            self->onFramebufferResize(width, height);
        }
    });
    glfwSetMouseButtonCallback(m_window, [](GLFWwindow* w, int button, int action, int mods) {
        if (auto* self = FromWindow(w)) {
            self->onMouseButton(button, action, mods);
        }
    });
    glfwSetCursorPosCallback(m_window, [](GLFWwindow* w, double x, double y) {
        if (auto* self = FromWindow(w)) {
            self->onCursorPos(x, y);
        }
    });
    glfwSetScrollCallback(m_window, [](GLFWwindow* w, double xOff, double yOff) {
        if (auto* self = FromWindow(w)) {
            self->onScroll(xOff, yOff);
        }
    });
    glfwSetKeyCallback(m_window, [](GLFWwindow* w, int key, int sc, int action, int mods) {
        if (auto* self = FromWindow(w)) {
            self->onKey(key, sc, action, mods);
        }
    });
}

void SampleModule::fitToScene(bool recompute)
{
    if (m_view == nullptr || m_cameraManipulator == nullptr) {
        return;
    }
    auto scene = m_view->GetScene();
    if (scene == nullptr) {
        return;
    }
    const auto& aabb = scene->GetAABB(recompute);
    if (aabb.IsValid()) {
        m_cameraManipulator->FitTo(aabb);
    }
}

void SampleModule::onFramebufferResize(int width, int height)
{
    if (width <= 0 || height <= 0) {
        return;
    }
    const Size2u newSize{static_cast<uint32_t>(width), static_cast<uint32_t>(height)};
    if (newSize.width == m_windowSize.width && newSize.height == m_windowSize.height) {
        return;
    }

    if (m_graphicsAPI && m_view) {
        m_graphicsAPI->MakeContextCurrent();
        auto rt = m_view->GetRenderTarget();
        auto color = m_graphicsAPI->GetColorAttachment(rt);
        color.SetSize(newSize);
        auto depth = m_graphicsAPI->GetDepthAttachment(rt);
        if (depth.IsValid()) {
            depth.SetSize(newSize);
        }
        rt.SetSize(newSize);
        rt.Build();
        m_graphicsAPI->DoneContextCurrent();
    }

    if (m_cameraManipulator) {
        m_cameraManipulator->SetViewport(newSize);
    }

    m_windowSize = newSize;
}

void SampleModule::onMouseButton(int button, int action, int /*mods*/)
{
    if (button < 0 || button > 31) {
        return;
    }
    const uint32_t bit = 1u << button;
    if (action == GLFW_PRESS) {
        m_mouseState.pressedButtons |= bit;
        glfwGetCursorPos(m_window, &m_mouseState.lastX, &m_mouseState.lastY);
        if (button == GLFW_MOUSE_BUTTON_RIGHT && m_cameraManipulator) {
            m_cameraManipulator->BeginRotate();
        }
    } else if (action == GLFW_RELEASE) {
        m_mouseState.pressedButtons &= ~bit;
        if (button == GLFW_MOUSE_BUTTON_RIGHT && m_cameraManipulator) {
            m_cameraManipulator->EndRotate();
        }
    }
}

void SampleModule::onCursorPos(double xPos, double yPos)
{
    if (m_cameraManipulator == nullptr) {
        return;
    }
    Vector2f delta{static_cast<float>(xPos - m_mouseState.lastX), static_cast<float>(yPos - m_mouseState.lastY)};

    const bool rightHeld = (m_mouseState.pressedButtons & kRightButtonBit) != 0u;
    const bool middleHeld = (m_mouseState.pressedButtons & kMiddleButtonBit) != 0u;
    const bool ctrlHeld = glfwGetKey(m_window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS ||
                          glfwGetKey(m_window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS;

    if (rightHeld) {
        m_cameraManipulator->RotateTrack(delta, 3.0f);
    }
    if (middleHeld && ctrlHeld) {
        m_cameraManipulator->AlongAxisMove(delta, 0.01f);
    } else if (middleHeld) {
        m_cameraManipulator->FlyMove(delta, 0.01f);
    }

    m_mouseState.lastX = xPos;
    m_mouseState.lastY = yPos;
}

void SampleModule::onScroll(double /*xOffset*/, double yOffset)
{
    if (m_cameraManipulator == nullptr) {
        return;
    }
    const float step = static_cast<float>(yOffset) * 120.0f;
    const bool shiftHeld = glfwGetKey(m_window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ||
                           glfwGetKey(m_window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS;
    if (shiftHeld) {
        m_cameraManipulator->Zoom(step);
    } else {
        m_cameraManipulator->Dolly(step, 0.01f);
    }
}

void SampleModule::onKey(int key, int /*scancode*/, int action, int mods)
{
    if (action != GLFW_PRESS && action != GLFW_REPEAT) {
        return;
    }
    if (m_cameraManipulator == nullptr) {
        return;
    }
    const float magnitude = (action == GLFW_REPEAT) ? 5.0f : 10.0f;
    switch (key) {
    case GLFW_KEY_W:
    case GLFW_KEY_UP:
        m_cameraManipulator->Dolly(magnitude);
        break;
    case GLFW_KEY_S:
    case GLFW_KEY_DOWN:
        m_cameraManipulator->Dolly(-magnitude);
        break;
    case GLFW_KEY_F:
        fitToScene((mods & GLFW_MOD_SHIFT) != 0);
        break;
    case GLFW_KEY_ESCAPE:
        glfwSetWindowShouldClose(m_window, GLFW_TRUE);
        break;
    default:
        break;
    }
}

} // namespace CS
