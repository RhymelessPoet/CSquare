#pragma once
#include <memory>

namespace CS
{

class View;

class IRenderSample
{
public:
    IRenderSample(/* args */) = default;

    virtual void Initialize(std::shared_ptr<View> view) = 0;
};

} // namespace CS
