#pragma once

namespace CS
{
class EngineController
{
public:
    EngineController(/* args */);
    ~EngineController() = default;

    void exit() { m_isToExit = true; }
    bool isToExit() const { return m_isToExit; }

private:
    bool m_isToExit{false};
};

} // namespace CS
