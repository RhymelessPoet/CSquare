#include "IRenderable.h"
#include "IRenderSystem.h"

namespace CS
{

std::shared_ptr<GraphicsAPI> IRenderable::getGraphicsAPI() const
{
    return dynamic_cast<IRenderSystem*>(system())->GetGraphicsAPI();
}

} // namespace CS
