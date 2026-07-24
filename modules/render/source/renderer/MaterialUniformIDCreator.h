#pragma once
#include <string>

namespace CS
{

class MaterialUniformIDCreator
{
public:
    MaterialUniformIDCreator(/* args */) = default;

    std::string GetUniformIdentifier(uint16_t materialID, uint32_t instanceID, std::string_view name);
};

} // namespace CS
