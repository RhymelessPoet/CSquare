#pragma once
#include "IGraphicsCommand.h"
#include "base/Color.h"

namespace CS
{

class Command_BeginPass : public IGraphicsCommand
{
public:
    Command_BeginPass(size_t renderTarget) : m_renderTarget(renderTarget) {}

    virtual bool Execute(std::shared_ptr<GraphicsGLImpl>& graphicsAPI) override;

private:
    size_t m_renderTarget{-1u};
};

class Command_EndPass : public IGraphicsCommand
{
public:
    Command_EndPass() {}

    virtual bool Execute(std::shared_ptr<GraphicsGLImpl>& graphicsAPI) override;
};

class Command_Clear final : public IGraphicsCommand
{
public:
    Command_Clear(std::optional<Color> clearColor) : m_color(clearColor) {}

    virtual bool Execute(std::shared_ptr<GraphicsGLImpl>& graphicsAPI) override;

private:
    std::optional<Color> m_color;
    std::optional<float> m_depth;
};

} // namespace CS
