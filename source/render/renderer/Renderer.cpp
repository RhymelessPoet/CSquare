#include "Renderer.h"
#include "GraphicsResourceManager.h"
#include "RenderContext.h"
#include "graphics/GraphicsAPI.h"
#include "renderer/MaterialCompiler.h"
#include "scene/View.h"
#include "scene/ViewGraph.h"

namespace CS
{
Renderer::Renderer(std::shared_ptr<GraphicsAPI> api) : m_graphicAPI(std::move(api))
{
    m_renderContext = std::make_unique<RenderContext>(m_graphicAPI);
    m_resourceManager = std::make_shared<GraphicsResourceManager>(m_graphicAPI);
    m_renderContext->SetMaterialCompiler(std::make_unique<MaterialCompiler>(m_graphicAPI, m_resourceManager));
}

Renderer::~Renderer() {}

void Renderer::Render(const ViewGraph& graph)
{
    graph.GetMainView()->Render(*m_renderContext);
    m_resourceManager->UpdateResources();
    m_graphicAPI->SubmitCommandBuffer(m_renderContext->GetCommandBuffer());
}

} // namespace CS
