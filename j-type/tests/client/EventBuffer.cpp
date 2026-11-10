/*
** EPITECH PROJECT, 2026
** j-type/tests/client/EventBuffer
** File description:
** EventBuffer unit tests
*/

#include <criterion/criterion.h>

#include <cstdint>
#include <variant>
#include <vector>

#include "jtype/common/Messages.hpp"
#include "world/EventBuffer.hpp"

using jtype::DestroyMessage;
using jtype::EventBuffer;
using jtype::GameOverEventMessage;
using jtype::Message;
using jtype::WelcomeMessage;

Test(client_event_buffer, releases_the_events_of_the_steps_drawn)
{
    EventBuffer events;

    events.push(DestroyMessage{12, 1});
    events.push(DestroyMessage{10, 2});
    events.push(GameOverEventMessage{11});
    events.push(WelcomeMessage{0});

    const std::vector<Message> drawn = events.release(11);

    cr_assert_eq(drawn.size(), 2U);
    cr_assert(std::get<DestroyMessage>(drawn[0]) == (DestroyMessage{10, 2}));
    cr_assert(std::holds_alternative<GameOverEventMessage>(drawn[1]));
    cr_assert_eq(events.release(11).size(), 0U);
    cr_assert_eq(events.release(12).size(), 1U);
}

Test(client_event_buffer, keeps_the_order_of_the_events_of_a_step)
{
    EventBuffer events;

    events.push(DestroyMessage{5, 1});
    events.push(DestroyMessage{5, 2});

    const std::vector<Message> drawn = events.release(5);

    cr_assert_eq(std::get<DestroyMessage>(drawn[0]).id, 1U);
    cr_assert_eq(std::get<DestroyMessage>(drawn[1]).id, 2U);
}

Test(client_event_buffer, drops_the_oldest_event_when_full)
{
    EventBuffer events;

    for (std::uint32_t tick = 0; tick <= jtype::MAX_PENDING_EVENTS; ++tick) {
        events.push(DestroyMessage{tick, tick});
    }

    const std::vector<Message> drawn = events.release(jtype::MAX_PENDING_EVENTS);

    cr_assert_eq(drawn.size(), jtype::MAX_PENDING_EVENTS);
    cr_assert_eq(std::get<DestroyMessage>(drawn.front()).tick, 1U);
}
