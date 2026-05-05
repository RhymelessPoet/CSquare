#pragma once
#include <UDRefl/Basic.hpp>
#include <typeinfo>

namespace CS
{
// Runtime bridge between C++ RTTI (std::type_info) and UDRefl Ubpa::Type.
// Populated by auto-generated Register_<T>() functions so that given a
// polymorphic base pointer we can resolve the concrete UDRefl Type without
// having to re-derive a mangled name at runtime.
class MetaTypeIndex
{
public:
    static void Register(const std::type_info& ti, Ubpa::Type type);
    static Ubpa::Type Resolve(const std::type_info& ti);
};

} // namespace CS
