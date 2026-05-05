#include "ComponentModel.h"

#include "ReflectedAny.h"
#include "scene/IComponent.h"
#include "scene/MetaTypeIndex.h"

#include <UDRefl/ranges/FieldRange.hpp>

#include <typeinfo>

namespace CSEditor
{

ComponentModel::ComponentModel(CS::IComponent* component) : m_component(component)
{
    build();
}

ComponentModel::~ComponentModel() = default;

void ComponentModel::build()
{
    m_properties.clear();
    m_valid = false;

    if (m_component == nullptr) {
        return;
    }

    Ubpa::Type type = CS::MetaTypeIndex::Resolve(typeid(*m_component));
    if (!type.Valid()) {
        return;
    }

    m_typeName = std::string{type.GetName()};
    m_view = Ubpa::UDRefl::ObjectView{type, static_cast<void*>(m_component)};

    // Only walk directly-declared, owned fields. Static / virtual / dynamic
    // fields are filtered out for the MVP.
    Ubpa::UDRefl::FieldRange range{m_view, Ubpa::UDRefl::FieldFlag::Owned};
    for (const auto& [name, info] : range) {
        auto fieldView = info.fieldptr.Var(static_cast<void*>(m_component));

        PropertyType propType = ClassifyType(fieldView);

        std::string uiName{name.GetView()};

        auto getter = [fieldView]() -> std::any { return ObjectViewToAny(fieldView); };

        const bool writable = (propType != PropertyType::Unknown);

        PropertyItem::Setter setter;
        if (writable) {
            // Prefer a registered setter method (e.g. "SetPosition" for
            // UIName "Position"). This lets the owning component run
            // side-effects such as dirtying cached matrices. When no such
            // method exists, fall back to a direct field write.
            std::string setterName = "Set" + uiName;
            Ubpa::UDRefl::ObjectView ownerView = m_view;
            setter = [ownerView, fieldView, setterName](const std::any& v) -> bool {
                return InvokeSetterOrWriteField(ownerView, fieldView, setterName, v);
            };
        }

        m_properties.emplace_back(std::move(uiName), propType, writable, std::move(getter), std::move(setter));
    }

    m_valid = true;
}

} // namespace CSEditor
