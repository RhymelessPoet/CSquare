#include "Engine.h"
#include "EngineController.h"
#include "core/IModule.h"

namespace CS
{
Engine& Engine::Instance()
{
    static Engine engine;
    return engine;
}
Engine::Engine()
{
    m_controller = std::make_shared<EngineController>();
}

Engine::~Engine() {}

void Engine::Run()
{
    for (auto module : m_modules) {
        module->Initialize();
    }

    while (!m_controller->isToExit()) {
        for (auto module : m_modules) {
            module->Update();
        }
    }
}

} // namespace CS
