#pragma once

#include "IRenderSystem.h"

namespace CS
{
class MeshRenderer;

class MeshRenderSystem : public IRenderSystem
{
public:
    MeshRenderSystem(std::shared_ptr<GraphicsAPI> graphicsAPI);
    ~MeshRenderSystem() override;

    virtual void OnUpdate() override;
    virtual void OnRender(RenderContext& context) override;
};

} // namespace CS
