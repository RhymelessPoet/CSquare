#pragma once
#include "ISystem.h"

namespace CS
{

class CameraSystem : public ISystem
{
public:
    CameraSystem() = default;
    ~CameraSystem() override = default;

    virtual void OnUpdate() override;
};

} // namespace CS
