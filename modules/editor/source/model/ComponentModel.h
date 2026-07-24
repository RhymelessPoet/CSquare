#pragma once
#include "PropertyItem.h"

#include <UDRefl/Object.hpp>

#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace CS
{
class IComponent;
} // namespace CS

namespace CSEditor
{

// Pure C++ domain model. Wraps one CS::IComponent*, introspects it with
// UDRefl and exposes its properties as type-erased PropertyItem instances.
// Zero Qt dependency — the Qt adapter layer (QuickComponentModel) is the
// only consumer that projects this into a QAbstractItemModel.
class ComponentModel
{
public:
    explicit ComponentModel(CS::IComponent* component);
    ~ComponentModel();

    ComponentModel(const ComponentModel&) = delete;
    ComponentModel& operator=(const ComponentModel&) = delete;

    // Returns true if UDRefl reflection was resolved for this component.
    bool IsValid() const { return m_valid; }

    // The UDRefl-registered type name (e.g. "CS::Transform"). Stable for
    // the lifetime of this object.
    std::string_view GetTypeName() const { return m_typeName; }

    // Property access.
    size_t GetPropertyCount() const { return m_properties.size(); }
    const PropertyItem& GetProperty(size_t i) const { return m_properties[i]; }
    PropertyItem& GetProperty(size_t i) { return m_properties[i]; }

private:
    void build();

    CS::IComponent* m_component{nullptr};
    Ubpa::UDRefl::ObjectView m_view{};
    std::string m_typeName;
    std::vector<PropertyItem> m_properties;
    bool m_valid{false};
};

} // namespace CSEditor
