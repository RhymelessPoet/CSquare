#include "GraphicsCommands.h"
#include "graphics/opengl/GraphicsGLImpl.h"

namespace CS
{

bool Command_BeginPass::Execute(std::shared_ptr<GraphicsGLImpl>& graphicsAPI)
{
    auto renderTarget = graphicsAPI->GetResourceDescriptor<RenderTargetDescriptor>(m_renderTarget);
    if (renderTarget != nullptr) {
        graphicsAPI->BindRenderTarget(renderTarget);
        return true;
    }
    return false;
}

bool Command_EndPass::Execute(std::shared_ptr<GraphicsGLImpl>& graphicsAPI)
{
    return false;
}

bool Command_Clear::Execute(std::shared_ptr<GraphicsGLImpl>& graphicsAPI)
{
    return graphicsAPI->Clear(m_color, m_depth);
}

} // namespace CS
