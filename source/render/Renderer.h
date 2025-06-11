#pragma once
#include "graphics/GraphicsAPI.h"
#include <memory>

namespace CS
{
class View;
class Renderer
{
public:
    Renderer(std::shared_ptr<GraphicsAPI> api);
    ~Renderer();

    void Render(std::shared_ptr<View> view);

    std::shared_ptr<GraphicsAPI> GetGraphicsAPI() { return m_graphicAPI; }

private:
    std::shared_ptr<GraphicsAPI> m_graphicAPI;
};

} // namespace CS
