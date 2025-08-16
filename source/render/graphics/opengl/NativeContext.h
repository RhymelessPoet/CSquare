#pragma once

struct GLFWwindow;

struct HGLRC__;
typedef HGLRC__* HGLRC;

namespace CS
{
class INativeContext
{
public:
    INativeContext() = default;
    virtual ~INativeContext() = 0 {};
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

} // namespace CS
