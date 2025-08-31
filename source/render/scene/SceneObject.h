#pragma once
#include <cassert>
#include <memory>
#include <vector>

namespace CS
{
class IComponent;
class SceneObject : public std::enable_shared_from_this<SceneObject>
{
public:
    SceneObject();
    ~SceneObject();

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
    bool AddChild(std::shared_ptr<SceneObject> child);
    bool RemoveChild(std::shared_ptr<SceneObject> child);

    std::shared_ptr<SceneObject> GetParent() const { return m_parent.lock(); }

private:
    std::weak_ptr<SceneObject> m_parent;
    std::vector<std::shared_ptr<SceneObject>> m_children;
    std::vector<std::unique_ptr<IComponent>> m_components;
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
