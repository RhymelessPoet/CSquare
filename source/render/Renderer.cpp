#include "Renderer.h"
#include "View.h"

namespace CS
{
Renderer::Renderer(std::shared_ptr<GraphicsAPI> api) : m_graphicAPI(std::move(api)) {}

Renderer::~Renderer() {}

void Renderer::Render(std::shared_ptr<View> view) {}

} // namespace CS
