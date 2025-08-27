#pragma once
#include <memory>

namespace CS
{
class View;
class RenderContext;
class GraphicsAPI;
class Renderer
{
public:
    Renderer(std::shared_ptr<GraphicsAPI> api);
    ~Renderer();

    void Render(std::shared_ptr<View> view);

    std::shared_ptr<GraphicsAPI> GetGraphicsAPI() { return m_graphicAPI; }

private:
    std::shared_ptr<GraphicsAPI> m_graphicAPI;
    std::unique_ptr<RenderContext> m_renderContext;
};

} // namespace CS
