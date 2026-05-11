#pragma once
#include "LightType.h"
#include "core/event/IEventListener.h"
#include "utils/AxisAlignedBoundingBox.h"
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace CS
{
class IRenderSystem;
class RenderContext;
class SceneObject;
class Camera;
class SceneObjectComposer;
class Material;
class IRenderable;
class NewGeometryNode;
class NewLight;
class Light;
class Scene : public std::enable_shared_from_this<Scene>, public IEventListener
{
public:
    Scene(std::shared_ptr<SceneObjectComposer> composer, std::string_view name = "");
    ~Scene();

    void OnRender(RenderContext& context);

    std::shared_ptr<SceneObject> GetRoot() { return m_root; }

    std::shared_ptr<Camera> CreateCamera();

    std::shared_ptr<SceneObject> CreateSceneObject(std::shared_ptr<SceneObject> parent = nullptr);

    std::shared_ptr<SceneObjectComposer> GetComposer() const { return m_composer; }

    const AABB& GetAABB() const { return m_box; }
    const AABB& GetAABB(bool reCompute = false);

    std::vector<std::shared_ptr<SceneObject>> GetLights(ELightType type) const;

    std::unique_ptr<IEvent> OnEvent(std::unique_ptr<IEvent> event) override;

private:
    void traverseWith(std::shared_ptr<SceneObject> object,
                      const std::function<void(std::shared_ptr<SceneObject>)>& func);
    void collectRenderables(std::shared_ptr<SceneObject> object);

    std::unique_ptr<IEvent> onEvent(NewGeometryNode* event);
    std::unique_ptr<IEvent> onEvent(NewLight* event);

    void updateShadowCamera(Camera& camera, const Light& light, const std::shared_ptr<Camera>& observerCamera);

private:
    std::string m_name;
    AxisAlignedBoundingBox m_box;
    std::shared_ptr<SceneObject> m_root;
    std::vector<std::weak_ptr<SceneObject>> m_lights;
    std::shared_ptr<SceneObjectComposer> m_composer;
    std::map<uint16_t, std::shared_ptr<Material>> m_materials;
    std::vector<IRenderable*> m_renderables;
};

} // namespace CS
