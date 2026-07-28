#pragma once
#include "Color.h"
#include "GraphicsResourceParameters.h"
#include "IGraphicsCommand.h"


namespace CS
{

class Command_BeginPass final : public IGraphicsCommand
{
public:
    Command_BeginPass(size_t renderTarget) : m_renderTarget(renderTarget) {}

    virtual bool Execute(std::shared_ptr<GraphicsGLImpl>& graphicsAPI) override;

private:
    size_t m_renderTarget{-1u};
};

class Command_EndPass final : public IGraphicsCommand
{
public:
    Command_EndPass() {}

    virtual bool Execute(std::shared_ptr<GraphicsGLImpl>& graphicsAPI) override;
};

class Command_Clear final : public IGraphicsCommand
{
public:
    Command_Clear(std::optional<Color> clearColor, std::optional<float> clearDepth)
        : m_color(clearColor), m_depth(clearDepth)
    {}

    virtual bool Execute(std::shared_ptr<GraphicsGLImpl>& graphicsAPI) override;

private:
    std::optional<Color> m_color;
    std::optional<float> m_depth;
};

class Command_BindInputAssembly : public IGraphicsCommand
{
public:
    Command_BindInputAssembly(size_t inputAssembly) : m_inputAssembly(inputAssembly) {}

    virtual bool Execute(std::shared_ptr<GraphicsGLImpl>& graphicsAPI) override;

private:
    size_t m_inputAssembly{-1u};
};
class Command_BindPipeline : public IGraphicsCommand
{
public:
    Command_BindPipeline(size_t pipeline) : m_pipeline(pipeline) {}

    virtual bool Execute(std::shared_ptr<GraphicsGLImpl>& graphicsAPI) override;

private:
    size_t m_pipeline{-1u};
};

class Command_BindComputePipeline : public IGraphicsCommand
{
public:
    explicit Command_BindComputePipeline(size_t pipeline) : m_pipeline(pipeline) {}
    bool Execute(std::shared_ptr<GraphicsGLImpl>& graphicsAPI) override;

private:
    size_t m_pipeline{-1u};
};

class Command_BindShaderBindingSet : public IGraphicsCommand
{
public:
    Command_BindShaderBindingSet(size_t shaderBindingSet) : m_shaderBindingSet(shaderBindingSet) {}

    virtual bool Execute(std::shared_ptr<GraphicsGLImpl>& graphicsAPI) override;

private:
    size_t m_shaderBindingSet{-1u};
};

class Command_SetViewport : public IGraphicsCommand
{
public:
    Command_SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
        : m_x(x), m_y(y), m_width(width), m_height(height)
    {}

    virtual bool Execute(std::shared_ptr<GraphicsGLImpl>& graphicsAPI) override;

private:
    uint32_t m_x{0u}, m_y{0u}, m_width{0u}, m_height{0u};
};

class Command_DrawIndexed : public IGraphicsCommand
{
public:
    Command_DrawIndexed(uint32_t count, uint32_t indexOffset);

    virtual bool Execute(std::shared_ptr<GraphicsGLImpl>& graphicsAPI) override;

private:
    uint32_t m_count{0u};
    uint32_t m_indexOffset{0u};
};

class Command_Dispatch : public IGraphicsCommand
{
public:
    Command_Dispatch(uint32_t x, uint32_t y, uint32_t z) : m_x(x), m_y(y), m_z(z) {}
    bool Execute(std::shared_ptr<GraphicsGLImpl>& graphicsAPI) override;

private:
    uint32_t m_x, m_y, m_z;
};

class Command_MemoryBarrier : public IGraphicsCommand
{
public:
    explicit Command_MemoryBarrier(MemoryBarrier barriers) : m_barriers(barriers) {}
    bool Execute(std::shared_ptr<GraphicsGLImpl>& graphicsAPI) override;

private:
    MemoryBarrier m_barriers;
};

} // namespace CS
