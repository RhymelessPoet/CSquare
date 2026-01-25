#pragma once
#include <memory>

namespace CS
{
class View;
class RenderContext;
class GraphicsAPI;
class GraphicsResourceManager;
class ViewGraph;
class Renderer
{
public:
    Renderer(std::shared_ptr<GraphicsAPI> api);
    ~Renderer();

    void Render(const ViewGraph& graph);

    std::shared_ptr<GraphicsAPI> GetGraphicsAPI() { return m_graphicAPI; }

private:
    std::shared_ptr<GraphicsAPI> m_graphicAPI;
    std::unique_ptr<RenderContext> m_renderContext;
    std::shared_ptr<GraphicsResourceManager> m_resourceManager;
};

} // namespace CS
