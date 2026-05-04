#pragma once
#include "IComponent.h"
#include "core/event/IEvent.h"
#include <memory>

namespace CS
{

class SceneObject;
class GeometryNode;
class LightComponent;

class SceneObjectEvent : public IEvent
{
public:
    SceneObjectEvent(std::string_view name, IComponent* component) : IEvent(name), m_component(component) {}

    std::shared_ptr<SceneObject> GetSceneObject() const
    {
        return m_component == nullptr ? nullptr : m_component->GetSO();
    }

    IComponent* GetComponent() const { return m_component; }

private:
    IComponent* m_component;
};

class NewGeometryNode : public SceneObjectEvent
{
public:
    NewGeometryNode(IComponent* component, std::weak_ptr<GeometryNode> geometryNode)
        : SceneObjectEvent("NewGeometryNode", component), m_geometryNode(std::move(geometryNode))
    {}

    std::shared_ptr<GeometryNode> GetGeometryNode() const { return m_geometryNode.lock(); }

private:
    std::weak_ptr<GeometryNode> m_geometryNode;
};

class NewLight : public SceneObjectEvent
{
public:
    NewLight(IComponent* component) : SceneObjectEvent("NewLight", component) {}
};

} // namespace CS
