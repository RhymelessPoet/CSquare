#include "SceneObject.h"
#include "IComponent.h"

namespace CS
{

SceneObject::SceneObject() {}

SceneObject::~SceneObject() {}

bool SceneObject::AddComponent(std::unique_ptr<IComponent> component)
{
    if (component) {
        m_components.push_back(std::move(component));
    }
    return true;
}

void SceneObject::SetParent(std::shared_ptr<SceneObject> parent)
{
    if (!m_parent.expired()) {
        m_parent.lock()->RemoveChild(shared_from_this());
    }
    parent->AddChild(shared_from_this());
    m_parent = parent;
}

bool SceneObject::AddChild(std::shared_ptr<SceneObject> child)
{
    m_children.push_back(child);
    return true;
}

bool SceneObject::RemoveChild(std::shared_ptr<SceneObject> child)
{
    auto it = std::remove(m_children.begin(), m_children.end(), child);
    if (it != m_children.end()) {
        m_children.erase(it, m_children.end());
        return true;
    } else {
        return false; // Child not found
    }
}

} // namespace CS
