#include "RenderModule.h"
#include "graphics/GLRendererBuilder.h"
#include "graphics/GraphicsAPI.h"
#include "graphics/GraphicsResourceCache.h"
#include "renderer/Renderer.h"
#include "scene/Scene.h"
#include "scene/View.h"

#include <condition_variable>
#include <mutex>
#include <thread>

namespace CS
{
class RenderModuleImpl
{
public:
    std::shared_ptr<View> view;
    std::unique_ptr<Renderer> renderer;
    std::shared_ptr<GraphicsResourceCache> resourceCache;

    std::mutex mtx;
    std::condition_variable cv;
    bool ready = false;
};

RenderModule::RenderModule()
{
    m_impl = std::make_unique<RenderModuleImpl>();
    m_impl->resourceCache = std::make_shared<GraphicsResourceCache>();
}

RenderModule::~RenderModule() {}

void RenderModule::Initialize() {}

void RenderModule::Update()
{
    std::unique_lock<std::mutex> lock(m_impl->mtx);
    m_impl->cv.wait(lock, [=] { return !m_impl->ready; });
    m_impl->view->GetScene()->OnUpdate();
    m_impl->ready = true;
}

void RenderModule::Render()
{
    std::unique_lock<std::mutex> lock(m_impl->mtx);
    if (m_impl->ready) {
        m_impl->ready = false;
        m_impl->cv.notify_all();
        m_impl->renderer->Render(m_impl->view);
    }
}

std::shared_ptr<View> RenderModule::CreateView()
{
    m_impl->view = std::make_shared<View>();
    return m_impl->view;
}

void RenderModule::CreateRenderer(const GLRendererBuilder& builder)
{
    m_impl->renderer = builder.Build(m_impl->resourceCache);
}

std::shared_ptr<GraphicsAPI> RenderModule::GetGraphicsAPI(std::shared_ptr<View> view) const
{
    return m_impl->renderer->GetGraphicsAPI();
}

} // namespace CS
