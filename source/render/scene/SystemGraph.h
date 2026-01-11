#pragma once
#include "base/PImpl.h"
#include <memory>
#include <vector>

namespace CS
{

class ISystem;
class InvalidSystem;

class SystemGraph : public PImpl<SystemGraph>
{
public:
    SystemGraph();

    template <typename SystemType, typename... Args>
        requires std::derived_from<SystemType, ISystem>
    bool AddSystem(Args... args)
    {
        if (GetSystem<SystemType>().IsValid()) {
            return false; // System already exists
        }
        auto system = std::make_unique<SystemType>(std::forward<Args>(args)...);
        systems().push_back(std::move(system));
        return true;
    }

    template <typename SystemType, typename... Args>
        requires std::derived_from<SystemType, ISystem>
    ISystem& GetSystem()
    {
        for (auto& system : systems()) {
            if (auto castedSystem = dynamic_cast<SystemType*>(system.get())) {
                return *castedSystem;
            }
        }
        return InvalidSystem::Instance();
    }

    void OnUpdate();

private:
    void sort();
    std::vector<std::unique_ptr<ISystem>>& systems();
};

} // namespace CS
