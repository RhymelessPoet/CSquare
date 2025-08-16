#include "Renderer.h"
#include "RenderContext.h"
#include "View.h"
#include "graphics/GraphicsAPI.h"

namespace CS
{
Renderer::Renderer(std::shared_ptr<GraphicsAPI> api) : m_graphicAPI(std::move(api))
{
    m_renderContext = std::make_unique<RenderContext>(m_graphicAPI);
}

Renderer::~Renderer() {}

void Renderer::Render(std::shared_ptr<View> view)
{
    view->Render(*m_renderContext);
    m_graphicAPI->SubmitCommandBuffer(m_renderContext->GetCommandBuffer());
}

} // namespace CS
