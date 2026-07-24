#pragma once
#include <string>

namespace CS
{

class IEvent
{
public:
    IEvent(std::string_view name) : m_name(std::string(name)) {}
    virtual ~IEvent() = default;

    std::string_view GetName() const { return m_name; }

private:
    std::string m_name;
};

} // namespace CS
