#pragma once
#include "core/IModule.h"

namespace CS
{
class View;
class RenderTarget;
class GLRendererBuilder;
class RenderModuleImpl;
class GraphicsAPI;

class RenderModule : public IModule
{
public:
    RenderModule();
    ~RenderModule() override;

    virtual void Initialize() override;
    virtual void Update() override;

    virtual void SetEngineController(std::shared_ptr<EngineController> controller) override {}

    void Render();

    std::shared_ptr<View> CreateView();

    void CreateRenderer(const GLRendererBuilder& builder);

    std::shared_ptr<GraphicsAPI> GetGraphicsAPI(std::shared_ptr<View> view) const;

private:
    std::unique_ptr<RenderModuleImpl> m_impl;
};

} // namespace CS
