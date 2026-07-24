#pragma once

struct GLFWwindow;

struct HGLRC__;
typedef HGLRC__* HGLRC;
struct HWND__;
typedef HWND__* HWND;
struct HDC__;
typedef HDC__* HDC;

namespace CS
{
class INativeContext
{
public:
    INativeContext() = default;
    virtual ~INativeContext() = 0 {};

    virtual bool MakeCurrent() { return true; }
    virtual void DoneCurrent() {}
    virtual void SwapBuffers() {}
};

class GLFWContext : public INativeContext
{
public:
    GLFWContext();
    ~GLFWContext();

public:
    GLFWwindow* window{nullptr};
    HGLRC context{nullptr};
};

class WGLContext : public INativeContext
{
public:
    WGLContext(HGLRC _context);
    WGLContext();
    ~WGLContext();

public:
    HGLRC context{nullptr};

private:
    bool m_shouldDestroy{true};
};

// Creates an independent HGLRC bound to a native window's HDC, optionally
// sharing resources (texture/buffer/shader namespace) with an existing
// HGLRC (typically the one owned by the host UI framework, e.g. Qt RHI).
class WGLWindowContext : public INativeContext
{
public:
    WGLWindowContext(HWND hwnd, HGLRC sharedContext);
    ~WGLWindowContext() override;

    bool MakeCurrent() override;
    void DoneCurrent() override;
    void SwapBuffers() override;

    HGLRC GetHGLRC() const { return m_hglrc; }
    HDC GetHDC() const { return m_hdc; }

private:
    HWND m_hwnd{nullptr};
    HDC m_hdc{nullptr};
    HGLRC m_hglrc{nullptr};
};

} // namespace CS
