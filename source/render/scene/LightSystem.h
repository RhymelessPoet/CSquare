#pragma once
#include "ISystem.h"

namespace CS
{

class LightSystem : public ISystem
{
public:
    LightSystem() = default;
    ~LightSystem() override = default;

    void OnUpdate(SystemContext& context) override;

private:
    std::unique_ptr<IEvent> dispatch(IEventDispatcher* nextDispatcher, std::unique_ptr<IEvent> event) override;
};

} // namespace CS
