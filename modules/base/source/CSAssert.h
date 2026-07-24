#pragma once
#include <stdexcept>

namespace CS
{

inline void Assert(bool condition, const char* message = "Assertion failed")
{
    if (!condition) {
        throw std::runtime_error(message);
    }
}

} // namespace CS
