#pragma once
#include "ISystem.h"

namespace CS
{
class RenderContext;
class GraphicsAPI;

class IRenderSystem : public ISystem
{
public:
    IRenderSystem();
    virtual ~IRenderSystem() = default;

    virtual void OnRender(RenderContext& context) = 0;
    ;
};

} // namespace CS
