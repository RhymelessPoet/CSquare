#pragma once
#include "IEvent.h"
#include "Macros.h"
#include <memory>
#include <optional>
#include <queue>
#include <ranges>

namespace CS
{
class IEvent;
class IEventListener;

class IEventDispatcher
{
public:
    IEventDispatcher(/* args */) = default;
    virtual ~IEventDispatcher() = default;

    void Dispatch(IEventDispatcher* nextDispatcher = nullptr);

    template <typename EventType, typename... Args>
    void PushEvent(Args&&... args)
        requires(std::derived_from<EventType, IEvent>)
    {
        PushEvent(std::make_unique<EventType>(std::forward<Args>(args)...));
    }

    void PushEvent(std::unique_ptr<IEvent> event) { m_eventQueue.push(std::move(event)); }

private:
    virtual std::vector<IEventListener*> sift(IEvent* event) const = 0;
    virtual std::unique_ptr<IEvent> dispatch(IEventDispatcher* nextDispatcher, std::unique_ptr<IEvent> event) = 0;

private:
    std::queue<std::unique_ptr<IEvent>> m_eventQueue;
};

class InvalidEventDispatcher : public IEventDispatcher
{
public:
    static IEventDispatcher* Instance()
    {
        static InvalidEventDispatcher instance;
        return &instance;
    }
    CS_DELETE_COPY_MOVE(InvalidEventDispatcher);

private:
    InvalidEventDispatcher() = default;
    std::vector<IEventListener*> sift(IEvent* event) const override;
    std::unique_ptr<IEvent> dispatch(IEventDispatcher* nextDispatcher, std::unique_ptr<IEvent> event) override;
};

} // namespace CS
