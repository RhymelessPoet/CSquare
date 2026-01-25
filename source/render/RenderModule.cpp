#include "RenderModule.h"
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
class RenderModuleImpl
{
public:
    std::unique_ptr<Renderer> renderer;
    std::shared_ptr<GraphicsResourceCache> resourceCache;
    std::shared_ptr<SystemGraph> systemGraph;
    std::unique_ptr<ViewGraph> viewGraph;

    std::mutex mtx;
    std::condition_variable cv;
    bool ready = false;
};

RenderModule::RenderModule()
{
    m_impl = std::make_unique<RenderModuleImpl>();
    m_impl->resourceCache = std::make_shared<GraphicsResourceCache>();
    m_impl->systemGraph = std::make_shared<SystemGraph>();
    m_impl->viewGraph = std::make_unique<ViewGraph>();
}

RenderModule::~RenderModule() {}

void RenderModule::Initialize() {}

void RenderModule::Update()
{
    std::unique_lock<std::mutex> lock(m_impl->mtx);
    m_impl->cv.wait(lock, [=] { return !m_impl->ready; });
    m_impl->systemGraph->OnUpdate();
    m_impl->ready = true;
}

void RenderModule::Render()
{
    std::unique_lock<std::mutex> lock(m_impl->mtx);
    if (m_impl->ready) {
        m_impl->ready = false;
        m_impl->cv.notify_all();
        m_impl->renderer->Render(*m_impl->viewGraph);
    }
}

std::shared_ptr<View> RenderModule::GetMainView()
{
    return m_impl->viewGraph->GetMainView();
}

SystemGraph& RenderModule::GetSystemGraph()
{
    return *m_impl->systemGraph;
}

void RenderModule::CreateRenderer(const GLRendererBuilder& builder)
{
    m_impl->renderer = builder.Build(m_impl->resourceCache);
}

std::shared_ptr<SceneObjectComposer> RenderModule::GetSOComposer() const
{
    return std::shared_ptr<SceneObjectComposer>(new SceneObjectComposer(m_impl->systemGraph));
}

std::shared_ptr<GraphicsAPI> RenderModule::GetGraphicsAPI(std::shared_ptr<View> view) const
{
    return m_impl->renderer->GetGraphicsAPI();
}

const ViewGraph& RenderModule::GetViewGraph() const
{
    return *m_impl->viewGraph;
}

} // namespace CS
