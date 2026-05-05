#include "SceneObjectModel.h"

#include "scene/IComponent.h"
#include "scene/SceneObject.h"

namespace CSEditor
{

SceneObjectModel::SceneObjectModel() = default;
SceneObjectModel::~SceneObjectModel() = default;

void SceneObjectModel::SetSceneObject(std::shared_ptr<CS::SceneObject> so)
{
    if (m_so == so) {
        return;
    }
    m_so = std::move(so);
    rebuildComponents();
    notify();
}

void SceneObjectModel::Clear()
{
    if (!m_so && m_components.empty()) {
        return;
    }
    m_so.reset();
    m_components.clear();
    notify();
}

std::string SceneObjectModel::GetName() const
{
    return m_so ? m_so->GetName() : std::string{};
}

bool SceneObjectModel::SetName(std::string_view name)
{
    if (!m_so) {
        return false;
    }
    m_so->SetName(name);
    notify();
    return true;
}

bool SceneObjectModel::IsActive() const
{
    return m_so ? m_so->IsActive() : false;
}

bool SceneObjectModel::SetActive(bool active)
{
    if (!m_so) {
        return false;
    }
    m_so->SetActive(active);
    notify();
    return true;
}

void SceneObjectModel::AddListener(Listener cb)
{
    if (cb) {
        m_listeners.emplace_back(std::move(cb));
    }
}

void SceneObjectModel::rebuildComponents()
{
    m_components.clear();
    if (!m_so) {
        return;
    }
    const auto& comps = m_so->GetComponents();
    m_components.reserve(comps.size());
    for (const auto& comp : comps) {
        if (!comp) {
            continue;
        }
        auto model = std::make_unique<ComponentModel>(comp.get());
        if (!model->IsValid()) {
            // Skip components without UDRefl registration; they cannot be
            // introspected. They can still be added later once annotated.
            continue;
        }
        m_components.emplace_back(std::move(model));
    }
}

void SceneObjectModel::notify()
{
    for (auto& cb : m_listeners) {
        if (cb) {
            cb();
        }
    }
}

} // namespace CSEditor
