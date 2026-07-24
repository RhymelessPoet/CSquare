#pragma once
#include "ViewType.h"
#include "PImpl.h"
#include "event/IEventListener.h"
#include "graphics/RenderTarget.h"
#include <span>

namespace CS
{

class View;
class Material;
class Scene;
class IMaterialConfiguration;
class GraphicsAPI;
class RenderContext;
class SceneObject;

class ViewGraph final : public PImpl<ViewGraph, IEventListener>
{
public:
    ViewGraph(std::shared_ptr<GraphicsAPI> graphicsAPI);

    std::shared_ptr<View> GetMainView() const;

    void Recompute();

    std::shared_ptr<View> CreateView(EViewType type);
    std::shared_ptr<View> CreateView(EViewType type, RenderTarget target);

    std::unique_ptr<IEvent> OnEvent(std::unique_ptr<IEvent> event) override;

    void OnRender(RenderContext& context) const;

private:
    void onNewMaterialInScene(const std::shared_ptr<Material>& material, const std::shared_ptr<Scene>& scene);
    void onNewLightInScene(const std::shared_ptr<SceneObject>& sceneObject, const std::shared_ptr<Scene>& scene);
    std::shared_ptr<View> rearrangePredecessorView(const std::shared_ptr<Material>& material,
                                                   const std::shared_ptr<Scene>& scene);
    std::span<const std::shared_ptr<View>> getViews(std::shared_ptr<Scene> scene, const IMaterialConfiguration& config);

    void configShadowView(std::shared_ptr<View> view);
};

} // namespace CS
