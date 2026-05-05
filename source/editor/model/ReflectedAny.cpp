#include "ReflectedAny.h"

#include "base/math/Vector.h"

#include <UDRefl/Basic.hpp>
#include <UDRefl/Object.hpp>

#include <array>
#include <string>

namespace CSEditor
{
namespace
{
// Strip top-level CV / reference qualifiers so we can compare against the
// primitive Type_of<T> values registered by UDRefl.
Ubpa::UDRefl::ObjectView plainView(Ubpa::UDRefl::ObjectView view)
{
    return view.RemoveConstReference();
}

bool isType(Ubpa::UDRefl::ObjectView view, Ubpa::Type t)
{
    return plainView(view).GetType() == t;
}

template <typename T>
T* asPtr(Ubpa::UDRefl::ObjectView view)
{
    return static_cast<T*>(view.RemoveConst().GetPtr());
}
} // namespace

PropertyType ClassifyType(Ubpa::UDRefl::ObjectView view)
{
    if (!view.GetType().Valid()) {
        return PropertyType::Unknown;
    }

    if (isType(view, Ubpa::Type_of<bool>)) {
        return PropertyType::Bool;
    }
    if (isType(view, Ubpa::Type_of<int>) || isType(view, Ubpa::Type_of<unsigned int>) ||
        isType(view, Ubpa::Type_of<std::uint32_t>) || isType(view, Ubpa::Type_of<std::int32_t>))
    {
        return PropertyType::Int;
    }
    if (isType(view, Ubpa::Type_of<float>)) {
        return PropertyType::Float;
    }
    if (isType(view, Ubpa::Type_of<double>)) {
        return PropertyType::Double;
    }
    if (isType(view, Ubpa::Type_of<std::string>)) {
        return PropertyType::String;
    }
    if (isType(view, Ubpa::Type_of<CS::Vector3f>)) {
        return PropertyType::Vector3;
    }
    return PropertyType::Unknown;
}

std::any ObjectViewToAny(Ubpa::UDRefl::ObjectView view)
{
    auto v = plainView(view);
    if (!v.GetType().Valid() || v.GetPtr() == nullptr) {
        return {};
    }

    switch (ClassifyType(v)) {
    case PropertyType::Bool:
        return std::any{*asPtr<const bool>(v)};
    case PropertyType::Int:
        if (isType(v, Ubpa::Type_of<int>) || isType(v, Ubpa::Type_of<std::int32_t>)) {
            return std::any{*asPtr<const int>(v)};
        }
        if (isType(v, Ubpa::Type_of<unsigned int>) || isType(v, Ubpa::Type_of<std::uint32_t>)) {
            return std::any{static_cast<int>(*asPtr<const unsigned int>(v))};
        }
        return {};
    case PropertyType::Float:
        return std::any{*asPtr<const float>(v)};
    case PropertyType::Double:
        return std::any{*asPtr<const double>(v)};
    case PropertyType::String:
        return std::any{*asPtr<const std::string>(v)};
    case PropertyType::Vector3: {
        const auto& vec = *asPtr<const CS::Vector3f>(v);
        return std::any{std::array<float, 3>{vec[0], vec[1], vec[2]}};
    }
    case PropertyType::Unknown:
    default:
        return {};
    }
}

bool AnyToObjectView(Ubpa::UDRefl::ObjectView view, const std::any& value)
{
    auto v = plainView(view);
    if (!v.GetType().Valid() || v.GetPtr() == nullptr || !value.has_value()) {
        return false;
    }

    switch (ClassifyType(v)) {
    case PropertyType::Bool:
        if (value.type() == typeid(bool)) {
            *asPtr<bool>(v) = std::any_cast<bool>(value);
            return true;
        }
        return false;
    case PropertyType::Int:
        if (value.type() == typeid(int)) {
            const int iv = std::any_cast<int>(value);
            if (isType(v, Ubpa::Type_of<int>) || isType(v, Ubpa::Type_of<std::int32_t>)) {
                *asPtr<int>(v) = iv;
                return true;
            }
            if (isType(v, Ubpa::Type_of<unsigned int>) || isType(v, Ubpa::Type_of<std::uint32_t>)) {
                *asPtr<unsigned int>(v) = static_cast<unsigned int>(iv < 0 ? 0 : iv);
                return true;
            }
        }
        return false;
    case PropertyType::Float:
        if (value.type() == typeid(float)) {
            *asPtr<float>(v) = std::any_cast<float>(value);
            return true;
        }
        if (value.type() == typeid(double)) {
            *asPtr<float>(v) = static_cast<float>(std::any_cast<double>(value));
            return true;
        }
        return false;
    case PropertyType::Double:
        if (value.type() == typeid(double)) {
            *asPtr<double>(v) = std::any_cast<double>(value);
            return true;
        }
        if (value.type() == typeid(float)) {
            *asPtr<double>(v) = static_cast<double>(std::any_cast<float>(value));
            return true;
        }
        return false;
    case PropertyType::String:
        if (value.type() == typeid(std::string)) {
            *asPtr<std::string>(v) = std::any_cast<std::string>(value);
            return true;
        }
        return false;
    case PropertyType::Vector3:
        if (value.type() == typeid(std::array<float, 3>)) {
            const auto& arr = std::any_cast<const std::array<float, 3>&>(value);
            *asPtr<CS::Vector3f>(v) = CS::Vector3f{arr[0], arr[1], arr[2]};
            return true;
        }
        return false;
    case PropertyType::Unknown:
    default:
        return false;
    }
}

bool InvokeSetterOrWriteField(Ubpa::UDRefl::ObjectView owner,
                              Ubpa::UDRefl::ObjectView fieldView,
                              std::string_view methodName,
                              const std::any& value)
{
    if (!owner.GetType().Valid() || owner.GetPtr() == nullptr || methodName.empty()) {
        return AnyToObjectView(fieldView, value);
    }

    const Ubpa::Name name{methodName};
    const PropertyType propType = ClassifyType(fieldView);

    auto tryInvokeOne = [&](Ubpa::Type argType, auto&& packArg) -> bool {
        const Ubpa::Type argTypes[] = {argType};
        if (!owner.IsInvocable(name, argTypes).Valid()) {
            return false;
        }
        return packArg();
    };

    switch (propType) {
    case PropertyType::Bool:
        if (value.type() == typeid(bool)) {
            const bool v = std::any_cast<bool>(value);
            if (tryInvokeOne(Ubpa::Type_of<bool>, [&] {
                    owner.Invoke<void>(name, Ubpa::UDRefl::TempArgsView{v});
                    return true;
                }))
            {
                return true;
            }
        }
        break;
    case PropertyType::Int:
        if (value.type() == typeid(int)) {
            const int v = std::any_cast<int>(value);
            if (tryInvokeOne(Ubpa::Type_of<int>, [&] {
                    owner.Invoke<void>(name, Ubpa::UDRefl::TempArgsView{v});
                    return true;
                }))
            {
                return true;
            }
            const unsigned int uv = static_cast<unsigned int>(v < 0 ? 0 : v);
            if (tryInvokeOne(Ubpa::Type_of<unsigned int>, [&] {
                    owner.Invoke<void>(name, Ubpa::UDRefl::TempArgsView{uv});
                    return true;
                }))
            {
                return true;
            }
        }
        break;
    case PropertyType::Float: {
        float v = 0.0f;
        if (value.type() == typeid(float)) {
            v = std::any_cast<float>(value);
        } else if (value.type() == typeid(double)) {
            v = static_cast<float>(std::any_cast<double>(value));
        } else {
            break;
        }
        if (tryInvokeOne(Ubpa::Type_of<float>, [&] {
                owner.Invoke<void>(name, Ubpa::UDRefl::TempArgsView{v});
                return true;
            }))
        {
            return true;
        }
        break;
    }
    case PropertyType::Double: {
        double v = 0.0;
        if (value.type() == typeid(double)) {
            v = std::any_cast<double>(value);
        } else if (value.type() == typeid(float)) {
            v = static_cast<double>(std::any_cast<float>(value));
        } else {
            break;
        }
        if (tryInvokeOne(Ubpa::Type_of<double>, [&] {
                owner.Invoke<void>(name, Ubpa::UDRefl::TempArgsView{v});
                return true;
            }))
        {
            return true;
        }
        break;
    }
    case PropertyType::String:
        if (value.type() == typeid(std::string)) {
            const std::string& v = std::any_cast<const std::string&>(value);
            if (tryInvokeOne(Ubpa::Type_of<std::string>, [&] {
                    owner.Invoke<void>(name, Ubpa::UDRefl::TempArgsView{v});
                    return true;
                }))
            {
                return true;
            }
        }
        break;
    case PropertyType::Vector3:
        if (value.type() == typeid(std::array<float, 3>)) {
            const auto& arr = std::any_cast<const std::array<float, 3>&>(value);
            CS::Vector3f vec{arr[0], arr[1], arr[2]};
            if (tryInvokeOne(Ubpa::Type_of<CS::Vector3f>, [&] {
                    owner.Invoke<void>(name, Ubpa::UDRefl::TempArgsView{vec});
                    return true;
                }))
            {
                return true;
            }
        }
        break;
    case PropertyType::Unknown:
    default:
        break;
    }

    // Fallback: direct field write.
    return AnyToObjectView(fieldView, value);
}

} // namespace CSEditor
