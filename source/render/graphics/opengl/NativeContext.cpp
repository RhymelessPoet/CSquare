#include "NativeContext.h"
#define GLFW_EXPOSE_NATIVE_WGL
#include "GLFW/glfw3.h"
#include "GLFW/glfw3native.h"
#include "base/Logger.h"
#include <windows.h>

namespace CS
{

GLFWContext::GLFWContext()
{
    window = glfwCreateWindow(1, 1, "Window", NULL, NULL);
    glfwMakeContextCurrent(window);
    context = glfwGetWGLContext(window);
}

GLFWContext::~GLFWContext()
{
    glfwDestroyWindow(window);
}

WGLContext::WGLContext(HGLRC _context) : context(_context), m_shouldDestroy(false) {}

WGLContext::WGLContext() {}

WGLContext::~WGLContext()
{
    if (m_shouldDestroy) {
        // wglDeleteContext(context);
    }
}

WGLWindowContext::WGLWindowContext(HWND hwnd, HGLRC sharedContext) : m_hwnd(hwnd)
{
    if (m_hwnd == nullptr) {
        CS::LogError(CS::BuiltInChannels::Render(), "WGLWindowContext: null HWND");
        return;
    }

    m_hdc = ::GetDC(m_hwnd);
    if (m_hdc == nullptr) {
        CS::LogError(CS::BuiltInChannels::Render(), "WGLWindowContext: GetDC failed");
        return;
    }

    // Qt has already set the pixel format on this HWND's HDC; reuse it.
    // Only set a pixel format if it has not been set yet (e.g. standalone use).
    const int currentPF = ::GetPixelFormat(m_hdc);
    if (currentPF == 0) {
        PIXELFORMATDESCRIPTOR pfd = {};
        pfd.nSize = sizeof(pfd);
        pfd.nVersion = 1;
        pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
        pfd.iPixelType = PFD_TYPE_RGBA;
        pfd.cColorBits = 32;
        pfd.cDepthBits = 24;
        pfd.cStencilBits = 8;
        pfd.iLayerType = PFD_MAIN_PLANE;
        const int pf = ::ChoosePixelFormat(m_hdc, &pfd);
        if (pf == 0 || ::SetPixelFormat(m_hdc, pf, &pfd) == FALSE) {
            CS::LogError(CS::BuiltInChannels::Render(), "WGLWindowContext: SetPixelFormat failed");
            return;
        }
    }

    m_hglrc = ::wglCreateContext(m_hdc);
    if (m_hglrc == nullptr) {
        CS::LogError(CS::BuiltInChannels::Render(), "WGLWindowContext: wglCreateContext failed");
        return;
    }

    if (sharedContext != nullptr) {
        // wglShareLists requires neither context to be current when called.
        if (::wglShareLists(sharedContext, m_hglrc) == FALSE) {
            CS::LogWarning(CS::BuiltInChannels::Render(),
                           "WGLWindowContext: wglShareLists failed (continuing without sharing)");
        } else {
            CS::LogInfo(CS::BuiltInChannels::Render(), "WGLWindowContext: sharing resources with host HGLRC");
        }
    }
}

WGLWindowContext::~WGLWindowContext()
{
    if (m_hglrc != nullptr) {
        if (::wglGetCurrentContext() == m_hglrc) {
            ::wglMakeCurrent(nullptr, nullptr);
        }
        ::wglDeleteContext(m_hglrc);
        m_hglrc = nullptr;
    }
    if (m_hwnd != nullptr && m_hdc != nullptr) {
        ::ReleaseDC(m_hwnd, m_hdc);
        m_hdc = nullptr;
    }
}

bool WGLWindowContext::MakeCurrent()
{
    if (m_hdc == nullptr || m_hglrc == nullptr) {
        return false;
    }
    return ::wglMakeCurrent(m_hdc, m_hglrc) == TRUE;
}

void WGLWindowContext::DoneCurrent()
{
    if (::wglGetCurrentContext() == m_hglrc) {
        ::wglMakeCurrent(nullptr, nullptr);
    }
}

void WGLWindowContext::SwapBuffers()
{
    if (m_hdc != nullptr) {
        ::SwapBuffers(m_hdc);
    }
}

} // namespace CS
