#pragma once
#include "IComponent.h"
#include <memory>
#include <vector>

namespace CS
{

class SceneObject : public std::enable_shared_from_this<SceneObject>
{
public:
    SceneObject();
    ~SceneObject();

    template <typename T>
    IComponent& GetComponent()
    {
        for (const auto& component : m_components) {
            if (auto castedComponent = dynamic_cast<T*>(component.get())) {
                return *castedComponent;
            }
        }
        return InvalidComponent::Instance();
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

} // namespace CS
