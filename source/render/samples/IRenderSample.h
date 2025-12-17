#pragma once
#include "base/Macros.h"
#include "base/math/Vector.h"
#include <memory>
#include <vector>


namespace CS
{

class View;

class IRenderSample
{
public:
    static Vector3f RandomPosition(const Vector3f& minRange, const Vector3f& maxRange);
    static std::vector<Vector3f> RandomPositions(uint32_t count, const Vector3f& minRange, const Vector3f& maxRange);

    IRenderSample(/* args */) = default;

    virtual void Initialize(std::shared_ptr<View> view) = 0;

    virtual void OnUpdate() = 0;
};

} // namespace CS
