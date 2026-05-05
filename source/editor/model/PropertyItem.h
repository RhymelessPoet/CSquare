#pragma once
#include <any>
#include <functional>
#include <string>
#include <string_view>

namespace CSEditor
{

// Tags describing the kind of value carried by a reflected property.
// Keep this minimal and stable — QML editors dispatch on this enum.
enum class PropertyType
{
    Unknown,
    Bool,
    Int,
    Float,
    Double,
    String,
    Vector3,
    Matrix4,
};

// A type-erased handle to a single reflected property on a component.
// The domain layer builds these from UDRefl; the adapter layer consumes
// them to serve QAbstractItemModel role queries.
class PropertyItem
{
public:
    using Getter = std::function<std::any()>;
    using Setter = std::function<bool(const std::any&)>;

    PropertyItem(std::string uiName, PropertyType type, bool writable, Getter getter, Setter setter)
        : m_uiName(std::move(uiName)), m_type(type), m_writable(writable), m_getter(std::move(getter)),
          m_setter(std::move(setter))
    {}

    std::string_view GetUIName() const { return m_uiName; }
    PropertyType GetType() const { return m_type; }
    bool IsWritable() const { return m_writable && static_cast<bool>(m_setter); }

    std::any GetValue() const { return m_getter ? m_getter() : std::any{}; }

    bool SetValue(const std::any& v)
    {
        if (!IsWritable()) {
            return false;
        }
        return m_setter(v);
    }

private:
    std::string m_uiName;
    PropertyType m_type;
    bool m_writable;
    Getter m_getter;
    Setter m_setter;
};

} // namespace CSEditor
