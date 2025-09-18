#pragma once
#include "IRenderSystem.h"
#include <map>

namespace CS
{
class MeshRenderer;
class Material;
class MeshRenderSystem : public IRenderSystem
{
public:
    MeshRenderSystem() = default;
    ~MeshRenderSystem() = default;

    virtual void OnUpdate() override;
    virtual void OnRender(RenderContext& context) override;

private:
    std::map<uint16_t, std::shared_ptr<Material>> m_materials;
};

} // namespace CS
