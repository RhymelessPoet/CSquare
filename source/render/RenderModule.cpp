#include "RenderModule.h"
#include "RenderModuleContext.h"
#include "graphics/GLRendererBuilder.h"
#include "graphics/GraphicsAPI.h"
#include "graphics/GraphicsResourceCache.h"
#include "renderer/Renderer.h"
#include "scene/Scene.h"
#include "scene/SceneObjectComposer.h"
#include "scene/SystemGraph.h"
#include "scene/View.h"
#include "scene/ViewGraph.h"

#include <condition_variable>
#include <mutex>
#include <thread>

namespace CS
{
template <>
struct ImplData<RenderModule>
{
    ImplData()
    {
        resourceCache = std::make_shared<GraphicsResourceCache>();
        systemGraph = std::make_shared<SystemGraph>();
    }
    std::unique_ptr<Renderer> renderer;
    std::shared_ptr<GraphicsResourceCache> resourceCache;
    std::shared_ptr<SystemGraph> systemGraph;
    std::unique_ptr<ViewGraph> viewGraph;

    std::mutex mtx;
    std::condition_variable cv;
    bool ready = false;
};

RenderModule::RenderModule() : ImplBase() {}

RenderModule::~RenderModule() {}

void RenderModule::Initialize() {}

void RenderModule::Update()
{
    std::unique_lock<std::mutex> lock(impl().mtx);
    impl().cv.wait(lock, [=] { return !impl().ready; });
    RenderModuleContext context(this);
    impl().systemGraph->OnUpdate(context);
    Dispatch();
    impl().ready = true;
}

void RenderModule::Render()
{
    std::unique_lock<std::mutex> lock(impl().mtx);
    if (impl().ready) {
        impl().ready = false;
        impl().cv.notify_all();
        impl().renderer->Render(*impl().viewGraph);
    }
}

std::shared_ptr<View> RenderModule::GetMainView()
{
    if (impl().viewGraph == nullptr) {
        return nullptr;
    }
    return impl().viewGraph->GetMainView();
}

SystemGraph& RenderModule::GetSystemGraph()
{
    return *impl().systemGraph;
}

void RenderModule::CreateRenderer(const GLRendererBuilder& builder)
{
    impl().renderer = builder.Build(impl().resourceCache);
    impl().viewGraph = std::make_unique<ViewGraph>(impl().renderer->GetGraphicsAPI());
}

std::shared_ptr<SceneObjectComposer> RenderModule::GetSOComposer() const
{
    return std::shared_ptr<SceneObjectComposer>(new SceneObjectComposer(impl().systemGraph));
}

std::shared_ptr<GraphicsAPI> RenderModule::GetGraphicsAPI(std::shared_ptr<View> view) const
{
    return impl().renderer->GetGraphicsAPI();
}

std::vector<IEventListener*> RenderModule::sift(IEvent* event) const
{
    return {impl().viewGraph.get()};
}

std::unique_ptr<IEvent> RenderModule::dispatch(IEventDispatcher* nextDispatcher, std::unique_ptr<IEvent> event)
{
    return event;
}

} // namespace CS
