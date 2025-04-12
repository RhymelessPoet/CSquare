#pragma once

namespace CS
{
class IModule
{
public:
    virtual ~IModule();

    virtual void Update() = 0;
};

} // namespace CS
