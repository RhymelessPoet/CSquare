#pragma once
#include <string>

namespace CS
{

class UUID
{
public:
    UUID();

    void Regenerate();

    const std::string_view ID() const { return m_data; }

private:
    std::string m_data;
};

} // namespace CS

namespace std
{
template <>
struct hash<CS::UUID>
{
    size_t operator()(const CS::UUID& uuid) const { return hash<string_view>{}(uuid.ID()); }
};
} // namespace std
