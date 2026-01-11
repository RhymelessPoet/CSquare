#include "SystemGraph.h"
#include "ISystem.h"

namespace CS
{

template <>
struct ImplData<SystemGraph>
{
    std::vector<std::unique_ptr<ISystem>> systems;
};

SystemGraph::SystemGraph() : PImpl<SystemGraph>() {}

void SystemGraph::OnUpdate()
{
    for (const auto& system : systems()) {
        system->OnUpdate();
    }
}

void SystemGraph::sort() {}

std::vector<std::unique_ptr<ISystem>>& SystemGraph::systems()
{
    return impl().systems;
}

} // namespace CS
