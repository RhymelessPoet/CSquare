#pragma once
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
class Scene : public std::enable_shared_from_this<Scene>
{
public:
    Scene(std::shared_ptr<SceneObjectComposer> composer, std::string_view name = "");
    ~Scene();

    void OnRender(RenderContext& context);

    std::shared_ptr<SceneObject> GetRoot() { return m_root; }

    std::shared_ptr<Camera> CreateCamera();

    std::shared_ptr<SceneObject> CreateSceneObject(std::shared_ptr<SceneObject> parent = nullptr);

    std::shared_ptr<SceneObjectComposer> GetComposer() const { return m_composer; }

private:
    void collectRenderables(std::shared_ptr<SceneObject> object);

private:
    std::string m_name;
    std::shared_ptr<SceneObject> m_root;
    std::shared_ptr<SceneObjectComposer> m_composer;
    std::map<uint16_t, std::shared_ptr<Material>> m_materials;
    std::vector<IRenderable*> m_renderables;
};

} // namespace CS
