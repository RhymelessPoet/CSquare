#pragma once
#include <memory>

namespace CS
{
class EngineController;
class IModule
{
public:
    virtual ~IModule();

    virtual void Update() = 0;

    virtual void SetEngineController(std::shared_ptr<EngineController> controller) = 0;
};

} // namespace CS
