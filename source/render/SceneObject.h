#pragma once
#include "IComponent.h"
#include <memory>
#include <vector>

namespace CS
{

class SceneObject
{
public:
    SceneObject(/* args */);
    ~SceneObject();

    template <typename T>
    T& GetComponent()
    {
        for (const auto& component : m_components) {
            if (auto castedComponent = dynamic_cast<T*>(component.get())) {
                return *castedComponent;
            }
        }
        return InvalidComponent::Instance();
    }

    bool AddComponent(std::unique_ptr<IComponent> component);

private:
    std::vector<std::unique_ptr<IComponent>> m_components;
};

} // namespace CS
