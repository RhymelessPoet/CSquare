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
};

} // namespace CS
