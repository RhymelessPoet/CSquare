#pragma once

#include "IRenderSystem.h"

namespace CS
{
class MeshRenderer;

class MeshRenderSystem : public IRenderSystem
{
public:
    MeshRenderSystem() = default;
    ~MeshRenderSystem() = default;

    virtual void OnUpdate() override;
    virtual void OnRender(RenderContext& context) override;
};

} // namespace CS
