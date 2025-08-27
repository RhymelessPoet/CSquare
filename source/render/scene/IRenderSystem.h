#pragma once
#include "ISystem.h"

namespace CS
{
class RenderContext;
class GraphicsAPI;

class IRenderSystem : public ISystem
{
public:
    IRenderSystem(std::shared_ptr<GraphicsAPI> graphicsAPI);
    virtual ~IRenderSystem() = default;

    virtual void OnRender(RenderContext& context) = 0;

    std::shared_ptr<GraphicsAPI> GetGraphicsAPI() const { return m_graphicsAPI; }

private:
    std::shared_ptr<GraphicsAPI> m_graphicsAPI;
};

} // namespace CS
