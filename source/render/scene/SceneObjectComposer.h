#pragma once
#include "SystemGraph.h"
#include <memory>

namespace CS
{
class SceneObject;

class SceneObjectComposer
{
public:
    friend class RenderModule;

    template <typename ComponentType, typename... Args>
    bool AddComponent(std::shared_ptr<SceneObject> sceneObject, Args&&... args)
    {
        auto systemGraph = m_systemGraph.lock();
        if (systemGraph) {
            auto& system = systemGraph->template GetSystem<typename ComponentType::SystemType>();
            if (system.IsValid()) {
                system.template CreateComponent<ComponentType>(sceneObject, std::forward<Args>(args)...);
                return true;
            }
        }
        return false;
    }

    template <typename ComponentType>
    bool RemoveComponent(std::shared_ptr<SceneObject> sceneObject)
    {
        auto systemGraph = m_systemGraph.lock();
        if (systemGraph) {
            auto& system = systemGraph->template GetSystem<typename ComponentType::SystemType>();
            if (system.IsValid()) {
                system.template RemoveComponent<ComponentType>(sceneObject);
                return true;
            }
        }
        return false;
    }

private:
    SceneObjectComposer(std::shared_ptr<SystemGraph> systems);

private:
    std::weak_ptr<SystemGraph> m_systemGraph;
};

} // namespace CS
