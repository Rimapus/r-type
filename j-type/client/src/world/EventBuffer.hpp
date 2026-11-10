/*
** EPITECH PROJECT, 2026
** j-type/client/src/world/EventBuffer
** File description:
** EventBuffer header file
*/

#pragma once

#include <cstddef>
#include <cstdint>
#include <deque>
#include <vector>

#include "jtype/common/Messages.hpp"

namespace jtype {

// The events kept at most: a few steps of a busy game, so a client that stops drawing does not grow without end
inline constexpr std::size_t MAX_PENDING_EVENTS = 512;

/**
 * @brief Keeps the events of the server until the world drawn reaches their step, so a sound or a text comes with what
 *        appears on the screen: the world is drawn a few steps behind the newest snapshot.
 */
class EventBuffer
{
public:
    /**
     * @brief Keep an event until its step is drawn. Past MAX_PENDING_EVENTS, the oldest one is dropped.
     *
     * @param event An event, a message that getEventTick() gives a step; another message is ignored
     */
    void push(Message event);

    /**
     * @brief Take the events of the steps drawn so far.
     *
     * @param tick The step of the world drawn
     *
     * @return The events at or before that step, in the order of their steps, then of their arrival.
     */
    [[nodiscard]] std::vector<Message> release(std::uint32_t tick);

private:
    // The events not played yet, in the order of their steps
    std::deque<Message> _events;
};

} // namespace jtype
