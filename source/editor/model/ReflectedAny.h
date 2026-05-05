#pragma once
#include "PropertyItem.h"
#include <UDRefl/Object.hpp>

#include <string_view>

namespace CSEditor
{

// Classify a reflected field into one of the MVP PropertyType tags.
PropertyType ClassifyType(Ubpa::UDRefl::ObjectView view);

// Read a reflected field into a std::any using the conventions expected
// by the adapter layer:
//   Bool    -> bool
//   Int     -> int
//   Float   -> float
//   Double  -> double
//   String  -> std::string
//   Vector3 -> std::array<float, 3>
//   Matrix4 -> std::string (human readable dump; read-only)
//   Unknown -> empty std::any
std::any ObjectViewToAny(Ubpa::UDRefl::ObjectView view);

// Write a std::any back into the reflected field. Accepts the same mapping
// as ObjectViewToAny. Returns true if the write was applied.
bool AnyToObjectView(Ubpa::UDRefl::ObjectView view, const std::any& value);

// Write a property value, preferring a registered setter method on `owner`
// named `methodName` (typically "Set<UIName>") when one exists with a
// compatible single-parameter signature for the field's PropertyType.
// Falls back to AnyToObjectView(fieldView, value) when no such method is
// registered. This is what makes Transform-like components refresh their
// internal dirty state after an inspector edit.
bool InvokeSetterOrWriteField(Ubpa::UDRefl::ObjectView owner,
                              Ubpa::UDRefl::ObjectView fieldView,
                              std::string_view methodName,
                              const std::any& value);

} // namespace CSEditor
