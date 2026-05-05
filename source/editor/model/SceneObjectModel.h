#pragma once
#include "ComponentModel.h"

#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace CS
{
class SceneObject;
} // namespace CS

namespace CSEditor
{

// Pure C++ domain model for the currently-selected SceneObject.
// Holds a shared_ptr to the SceneObject plus one ComponentModel per
// component, and forwards name / active edits directly to the SceneObject.
// Observers (the Qt adapter) subscribe via AddListener and are notified
// whenever the selection or its properties change.
class SceneObjectModel
{
public:
    using Listener = std::function<void()>;

    SceneObjectModel();
    ~SceneObjectModel();

    SceneObjectModel(const SceneObjectModel&) = delete;
    SceneObjectModel& operator=(const SceneObjectModel&) = delete;

    // Selection.
    void SetSceneObject(std::shared_ptr<CS::SceneObject> so);
    void Clear();
    bool HasSelection() const { return static_cast<bool>(m_so); }

    // Name / active header.
    std::string GetName() const;
    bool SetName(std::string_view name);
    bool IsActive() const;
    bool SetActive(bool active);

    // Components.
    size_t GetComponentCount() const { return m_components.size(); }
    ComponentModel& GetComponent(size_t i) { return *m_components[i]; }
    const ComponentModel& GetComponent(size_t i) const { return *m_components[i]; }

    // Bumped every time the component set is rebuilt (selection swapped or
    // cleared). Value-only mutations (SetName / SetActive) do NOT bump it.
    // Observers cache this to decide whether their cached ComponentModel
    // pointers are still valid or must be rebuilt.
    size_t GetStructureRevision() const { return m_structureRevision; }

    // Observer registration. Listeners are invoked on every successful
    // mutation and every SetSceneObject / Clear call.
    void AddListener(Listener cb);

private:
    void rebuildComponents();
    void notify();

    std::shared_ptr<CS::SceneObject> m_so;
    std::vector<std::unique_ptr<ComponentModel>> m_components;
    std::vector<Listener> m_listeners;
    size_t m_structureRevision{0};
};

} // namespace CSEditor
