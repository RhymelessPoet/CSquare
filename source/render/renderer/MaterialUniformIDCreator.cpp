#include "MaterialUniformIDCreator.h"
#include <format>

namespace CS
{
std::string
MaterialUniformIDCreator::GetUniformIdentifier(uint16_t materialID, uint32_t instanceID, std::string_view name)
{
    return std::format("M{:x}I{:x}{}", materialID, instanceID, name);
}

} // namespace CS
