#pragma once
#include "CSMetaDefine.h"
#include <cassert>
#include <memory>
#include <string>
#include <vector>

namespace CS
{
class IComponent;
class Scene;
class [[Meta()]] SceneObject : public std::enable_shared_from_this<SceneObject>
{
public:
    friend void Register_SceneObject();

    explicit SceneObject(std::string_view name = "");
    ~SceneObject();

    void SetScene(const std::shared_ptr<Scene>& scene) { m_scene = scene; }
    std::shared_ptr<Scene> GetScene() const { return m_scene.lock(); }

    [[Method()]]
    void SetName(std::string_view name)
    {
        m_name = name;
    }
    [[Method()]]
    std::string GetName() const
    {
        return m_name;
    }

    [[Method()]]
    bool IsActive() const
    {
        return m_active;
    }
    [[Method()]]
    void SetActive(bool active)
    {
        m_active = active;
    }

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

    const std::vector<std::unique_ptr<IComponent>>& GetComponents() const { return m_components; }

    [[Method()]]
    void SetParent(std::shared_ptr<SceneObject> parent);
    [[Method()]]
    std::shared_ptr<SceneObject> GetParent() const
    {
        return m_parent.lock();
    }

    [[Method()]]
    bool AddChild(std::shared_ptr<SceneObject> child);
    [[Method()]]
    bool RemoveChild(std::shared_ptr<SceneObject> child);

    [[Method()]]
    const std::vector<std::shared_ptr<SceneObject>>& GetChildren() const
    {
        return m_children;
    }

private:
    [[Property(Setter = SetName, Getter = GetName, UIName = Name)]]
    std::string m_name;
    std::weak_ptr<Scene> m_scene;
    std::weak_ptr<SceneObject> m_parent;
    std::vector<std::shared_ptr<SceneObject>> m_children;
    std::vector<std::unique_ptr<IComponent>> m_components;
    [[Property(Setter = SetActive, Getter = IsActive, UIName = Active)]]
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
