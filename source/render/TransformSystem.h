#pragma once
#include "ISystem.h"

namespace CS
{

class TransformSystem : public ISystem
{
public:
    TransformSystem(/* args */) = default;
    ~TransformSystem() = default;

    virtual void OnUpdate() override;
};

} // namespace CS
