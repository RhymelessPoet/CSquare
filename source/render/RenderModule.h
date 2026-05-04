#pragma once
#include "base/PImpl.h"
#include "core/IModule.h"
#include "core/event/IEventDispatcher.h"

namespace CS
{
class View;
class Scene;
class RenderTarget;
class GLRendererBuilder;
class GraphicsAPI;
class SystemGraph;
class SceneObjectComposer;
class ViewGraph;

class RenderModule : public PImpl<RenderModule, IModule, IEventDispatcher>
{
public:
    RenderModule();
    ~RenderModule() override;

    virtual void Initialize() override;
    virtual void Update() override;

    virtual void SetEngineController(std::shared_ptr<EngineController> controller) override {}

    void Render();

    std::shared_ptr<View> GetMainView();

    SystemGraph& GetSystemGraph();
    void CreateRenderer(const GLRendererBuilder& builder);

    std::shared_ptr<SceneObjectComposer> GetSOComposer() const;

    std::shared_ptr<GraphicsAPI> GetGraphicsAPI(std::shared_ptr<View> view) const;

private:
    std::vector<IEventListener*> sift(IEvent* event) const override;
    std::unique_ptr<IEvent> dispatch(IEventDispatcher* nextDispatcher, std::unique_ptr<IEvent> event) override;
};

} // namespace CS
