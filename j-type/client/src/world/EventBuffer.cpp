/*
** EPITECH PROJECT, 2026
** j-type/client/src/world/EventBuffer
** File description:
** EventBuffer source file
*/

#include "world/EventBuffer.hpp"

#include <algorithm>
#include <optional>
#include <utility>

namespace jtype {

void EventBuffer::push(Message event)
{
    const std::optional<std::uint32_t> tick = getEventTick(event);

    if (!tick) {
        return;
    }
    if (this->_events.size() == MAX_PENDING_EVENTS) {
        this->_events.pop_front();
    }
    // After the events of the same step, so the events of a step keep the order the server sent them in
    const auto place = std::ranges::find_if(this->_events, [&tick](const Message &kept) {
        return isNewerTick(*getEventTick(kept), *tick);
    });

    this->_events.insert(place, std::move(event));
}

std::vector<Message> EventBuffer::release(std::uint32_t tick)
{
    std::vector<Message> released;

    while (!this->_events.empty() && !isNewerTick(*getEventTick(this->_events.front()), tick)) {
        released.push_back(std::move(this->_events.front()));
        this->_events.pop_front();
    }
    return released;
}

} // namespace jtype
