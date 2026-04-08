#pragma once
#include "IEvent.h"
#include <memory>

namespace CS
{

class IEventListener
{
public:
    virtual std::unique_ptr<IEvent> OnEvent(std::unique_ptr<IEvent> event) = 0;
};

} // namespace CS
