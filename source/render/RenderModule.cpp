#include "RenderModule.h"
#include "graphics/GLRendererBuilder.h"
#include "graphics/GraphicsAPI.h"
#include "graphics/GraphicsResourceCache.h"
#include "renderer/Renderer.h"
#include "scene/Scene.h"
#include "scene/View.h"

namespace CS
{
class RenderModuleImpl
{
public:
    std::shared_ptr<View> m_view;
    std::unique_ptr<Renderer> m_renderer;
    std::shared_ptr<GraphicsResourceCache> m_resourceCache;
};

RenderModule::RenderModule()
{
    m_impl = std::make_unique<RenderModuleImpl>();
    m_impl->m_resourceCache = std::make_shared<GraphicsResourceCache>();
}

RenderModule::~RenderModule() {}

void RenderModule::Initialize() {}

void RenderModule::Update()
{
    m_impl->m_view->GetScene()->OnUpdate();
}

void RenderModule::Render()
{
    m_impl->m_renderer->Render(m_impl->m_view);
}

std::shared_ptr<View> RenderModule::CreateView()
{
    m_impl->m_view = std::make_shared<View>();
    return m_impl->m_view;
}

void RenderModule::CreateRenderer(const GLRendererBuilder& builder)
{
    m_impl->m_renderer = builder.Build(m_impl->m_resourceCache);
}

std::shared_ptr<GraphicsAPI> RenderModule::GetGraphicsAPI(std::shared_ptr<View> view) const
{
    return m_impl->m_renderer->GetGraphicsAPI();
}

} // namespace CS
