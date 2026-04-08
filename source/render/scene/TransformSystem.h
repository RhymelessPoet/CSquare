#pragma once
#include "ISystem.h"

namespace CS
{

class TransformSystem : public ISystem
{
public:
    TransformSystem(/* args */) = default;
    ~TransformSystem() = default;

    void OnUpdate(SystemContext& context) override;
};

} // namespace CS
