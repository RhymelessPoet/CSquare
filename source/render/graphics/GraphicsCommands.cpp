#include "GraphicsCommands.h"
#include "GraphicsInputAssemblyDescriptor.h"
#include "GraphicsPipelineDescriptor.h"
#include "ShaderBindingSetDescriptor.h"
#include "graphics/GraphicsGLImpl.h"

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
    return true;
}

bool Command_Clear::Execute(std::shared_ptr<GraphicsGLImpl>& graphicsAPI)
{
    return graphicsAPI->Clear(m_color, m_depth);
}

bool Command_BindInputAssembly::Execute(std::shared_ptr<GraphicsGLImpl>& graphicsAPI)
{
    auto descriptor = graphicsAPI->GetResourceDescriptor<GraphicsInputAssemblyDescriptor>(m_inputAssembly);
    buildGraphicsResource(descriptor);
    return graphicsAPI->BindGraphicsInputAssembly(descriptor);
}

bool Command_BindPipeline::Execute(std::shared_ptr<GraphicsGLImpl>& graphicsAPI)
{
    auto descriptor = graphicsAPI->GetResourceDescriptor<GraphicsPipelineDescriptor>(m_pipeline);
    buildGraphicsResource(descriptor);
    return graphicsAPI->BindGraphicsPipeline(descriptor);
}

bool Command_BindShaderBindingSet::Execute(std::shared_ptr<GraphicsGLImpl>& graphicsAPI)
{
    auto descriptor = graphicsAPI->GetResourceDescriptor<ShaderBindingSetDescriptor>(m_shaderBindingSet);
    buildGraphicsResource(descriptor);
    return graphicsAPI->BindShaderBindingSet(descriptor);
}

Command_DrawIndexed::Command_DrawIndexed(uint32_t count, uint32_t indexOffset)
    : m_count(count), m_indexOffset(indexOffset)
{}

bool Command_DrawIndexed::Execute(std::shared_ptr<GraphicsGLImpl>& graphicsAPI)
{
    return graphicsAPI->DrawIndexed(m_count, m_indexOffset);
}

bool Command_SetViewport::Execute(std::shared_ptr<GraphicsGLImpl>& graphicsAPI)
{
    return graphicsAPI->SetViewport(m_x, m_y, m_width, m_height);
}

} // namespace CS
