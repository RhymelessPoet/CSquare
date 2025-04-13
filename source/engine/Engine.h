#pragma once
#include <memory>
#include <vector>

namespace CS
{

class IModule;
class EngineController;

class Engine final
{
public:
    Engine(/* args */);
    ~Engine();

    void Run();

    template <typename ModuleType, typename... Args>
    ModuleType& addModule(Args&&... args)
    {
        ModuleType* module = new ModuleType(std::forward<Args>(args)...);
        module->SetEngineController(m_controller);
        m_modules.push_back(module);

        return *module;
    }

private:
    std::vector<IModule*> m_modules;
    std::shared_ptr<EngineController> m_controller;
};

} // namespace CS
