#include "SceneObject.h"

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

} // namespace CS
