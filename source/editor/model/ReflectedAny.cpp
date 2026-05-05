#include "ReflectedAny.h"

#include "base/math/Matrix.h"
#include "base/math/Vector.h"

#include <UDRefl/Basic.hpp>
#include <UDRefl/Object.hpp>

#include <array>
#include <sstream>
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

std::string dumpMatrix(const CS::Matrix4f& m)
{
    std::ostringstream oss;
    for (uint32_t r = 0; r < 4; ++r) {
        for (uint32_t c = 0; c < 4; ++c) {
            if (c != 0) {
                oss << ' ';
            }
            oss << m[r][c];
        }
        if (r != 3) {
            oss << '\n';
        }
    }
    return oss.str();
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
    if (isType(view, Ubpa::Type_of<CS::Matrix4f>)) {
        return PropertyType::Matrix4;
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
    case PropertyType::Matrix4:
        return std::any{dumpMatrix(*asPtr<const CS::Matrix4f>(v))};
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
    case PropertyType::Matrix4:
    case PropertyType::Unknown:
    default:
        return false;
    }
}

} // namespace CSEditor
