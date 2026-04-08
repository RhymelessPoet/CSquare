#pragma once
#include "base/PImpl.h"
#include "core/event/IEventListener.h"
#include "graphics/RenderTarget.h"

namespace CS
{

class View;

class ViewGraph final : public PImpl<ViewGraph>, public IEventListener
{
public:
    ViewGraph(/* args */);

    std::shared_ptr<View> GetMainView() const;

    void Recompute();

    std::shared_ptr<View> CreateView();
    std::shared_ptr<View> CreateView(RenderTarget target);

    std::unique_ptr<IEvent> OnEvent(std::unique_ptr<IEvent> event) override;
};

} // namespace CS
