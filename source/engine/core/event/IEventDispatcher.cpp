#include "IEventDispatcher.h"
#include "IEventListener.h"
#include <iostream>

namespace CS
{
void IEventDispatcher::Dispatch(IEventDispatcher* nextDispatcher)
{

    while (!m_eventQueue.empty()) {
        auto event = std::move(m_eventQueue.front());
        m_eventQueue.pop();
        for (auto listener : sift(event.get())) {
            if (listener == nullptr) {
                continue;
            }
            event = listener->OnEvent(std::move(event));
            if (event == nullptr) {
                break; // Event has been consumed, stop dispatching
            }

            if (nextDispatcher == nullptr) {
                continue; // No next dispatcher, continue dispatching to listeners
            }
            event = dispatch(nextDispatcher, std::move(event));
            if (event == nullptr) {
                break; // Event has been consumed by the next dispatcher, stop dispatching
            }
        }
        if (event != nullptr && nextDispatcher != nullptr) {
            (void)dispatch(nextDispatcher, std::move(event));
        }
    }
}

std::vector<IEventListener*> InvalidEventDispatcher::sift(IEvent* event) const
{
    return std::vector<IEventListener*>();
}

std::unique_ptr<IEvent> InvalidEventDispatcher::dispatch(IEventDispatcher* nextDispatcher,
                                                         std::unique_ptr<IEvent> event)
{
    std::cerr << R"(Warning: Attempted to dispatch an event to an invalid event dispatcher. 
                 Event will be dispatched to the next dispatcher!)"
              << std::endl;
    nextDispatcher->PushEvent(std::move(event));
    return nullptr;
}

} // namespace CS
