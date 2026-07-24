#pragma once
#include <memory>
#include <optional>
#include <vector>

namespace CS
{

class IModule;
class EngineController;

class Engine final
{
public:
    static Engine& Instance();
    Engine();
    ~Engine();

    Engine(const Engine&) = delete;
    Engine& operator=(const Engine&) = delete;
    Engine(Engine&&) = delete;
    Engine& operator=(Engine&&) = delete;

    void Run();

    template <typename ModuleType, typename... Args>
    ModuleType& AddModule(Args&&... args)
    {
        ModuleType* module = new ModuleType(std::forward<Args>(args)...);
        module->SetEngineController(m_controller);
        m_modules.push_back(module);

        return *module;
    }

    template <typename ModuleType>
    std::optional<ModuleType*> GetModule()
    {
        for (auto& module : m_modules) {
            if (typeid(*module) == typeid(ModuleType)) {
                return dynamic_cast<ModuleType*>(module);
            }
        }
        return std::nullopt;
    }

private:
    std::vector<IModule*> m_modules;
    std::shared_ptr<EngineController> m_controller;
};

} // namespace CS
