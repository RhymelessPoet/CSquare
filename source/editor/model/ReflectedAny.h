#pragma once
#include "PropertyItem.h"
#include <UDRefl/Object.hpp>

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

} // namespace CSEditor
