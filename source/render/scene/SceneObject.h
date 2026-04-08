#pragma once
#include <cassert>
#include <memory>
#include <string>
#include <vector>

namespace CS
{
class IComponent;
class Scene;
class SceneObject : public std::enable_shared_from_this<SceneObject>
{
public:
    explicit SceneObject(std::string_view name = "");
    ~SceneObject();

    void SetScene(const std::shared_ptr<Scene>& scene) { m_scene = scene; }
    std::shared_ptr<Scene> GetScene() const { return m_scene.lock(); }

    void SetName(std::string_view name) { m_name = name; }
    std::string GetName() const { return m_name; }

    bool IsActive() const { return m_active; }
    void SetActive(bool active) { m_active = active; }

    template <typename T>
    T* GetComponent()
    {
        for (const auto& component : m_components) {
            if (auto castedComponent = dynamic_cast<T*>(component.get())) {
                return castedComponent;
            }
        }
        return nullptr;
    }

    bool AddComponent(std::unique_ptr<IComponent> component);

    void SetParent(std::shared_ptr<SceneObject> parent);
    std::shared_ptr<SceneObject> GetParent() const { return m_parent.lock(); }

    bool AddChild(std::shared_ptr<SceneObject> child);
    bool RemoveChild(std::shared_ptr<SceneObject> child);

    const std::vector<std::shared_ptr<SceneObject>>& GetChildren() const { return m_children; }

private:
    std::string m_name;
    std::weak_ptr<Scene> m_scene;
    std::weak_ptr<SceneObject> m_parent;
    std::vector<std::shared_ptr<SceneObject>> m_children;
    std::vector<std::unique_ptr<IComponent>> m_components;
    bool m_active{true};
};

template <typename T>
    requires std::is_base_of_v<IComponent, T>
inline T& GetComponent(std::shared_ptr<SceneObject>& so)
{
    auto component = so->GetComponent<T>();
    assert(component != nullptr);
    return *component;
}

template <typename T>
    requires std::is_base_of_v<IComponent, T>
inline const T& GetComponent(const std::shared_ptr<SceneObject>& so)
{
    auto component = so->GetComponent<T>();
    assert(component != nullptr);
    return *component;
}

} // namespace CS
