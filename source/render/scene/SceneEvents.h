#pragma once
#include "core/event/IEvent.h"
#include <memory>

namespace CS
{
class Material;
class Scene;

class SceneEvent : public IEvent
{
public:
    SceneEvent(std::string name, std::shared_ptr<Scene> scene) : IEvent(name), m_scene(scene) {}

    std::shared_ptr<Scene> GetScene() const { return m_scene.lock(); }

private:
    std::weak_ptr<Scene> m_scene;
};

class NewMaterialInScene : public SceneEvent
{
public:
    NewMaterialInScene(std::shared_ptr<Scene> scene, std::shared_ptr<Material> material)
        : SceneEvent("NewMaterialInScene", scene), m_material(material)
    {}

    std::shared_ptr<Material> GetMaterial() const { return m_material.lock(); }

private:
    std::weak_ptr<Material> m_material;
};

} // namespace CS
