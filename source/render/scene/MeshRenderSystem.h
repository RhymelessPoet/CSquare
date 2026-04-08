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

    void OnUpdate(SystemContext& context) override;

private:
    std::vector<IEventListener*> sift(IEvent* event) const override;
    std::unique_ptr<IEvent> dispatch(IEventDispatcher* nextDispatcher, std::unique_ptr<IEvent> event) override;
};

} // namespace CS
