#include "NativeContext.h"
#define GLFW_EXPOSE_NATIVE_WGL
#include "GLFW/glfw3.h"
#include "GLFW/glfw3native.h"
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
} // namespace CS
