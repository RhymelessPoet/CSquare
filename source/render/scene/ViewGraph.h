#pragma once
#include "base/PImpl.h"
#include "graphics/RenderTarget.h"

namespace CS
{

class View;

class ViewGraph final : public PImpl<ViewGraph>
{
public:
    ViewGraph(/* args */);

    std::shared_ptr<View> GetMainView() const;

    void Recompute();

    std::shared_ptr<View> CreateView();
    std::shared_ptr<View> CreateView(RenderTarget target);
};

} // namespace CS
