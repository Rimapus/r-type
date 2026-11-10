/*
** EPITECH PROJECT, 2026
** younity-engine/tests/core/EventBus
** File description:
** EventBus unit tests
*/

#include <criterion/criterion.h>

#include <vector>

#include "younity/core/EventBus.hpp"

using younity::EventBus;

namespace {

struct HitEvent
{
    int damage;
};

struct DeathEvent
{
    int score;
};

} // namespace

Test(core_event_bus, subscriber_receives_the_published_event)
{
    EventBus bus;
    int received = 0;

    bus.subscribe<HitEvent>([&received](const HitEvent &event) {
        received = event.damage;
    });
    bus.publish(HitEvent{10});
    cr_assert_eq(received, 10);
}

Test(core_event_bus, all_subscribers_are_called_in_order)
{
    EventBus bus;
    std::vector<int> order;

    bus.subscribe<HitEvent>([&order](const HitEvent &) {
        order.push_back(1);
    });
    bus.subscribe<HitEvent>([&order](const HitEvent &) {
        order.push_back(2);
    });
    bus.publish(HitEvent{10});
    cr_assert(order == (std::vector<int>{1, 2}));
}

Test(core_event_bus, subscriber_is_called_at_each_publish)
{
    EventBus bus;
    int calls = 0;

    bus.subscribe<HitEvent>([&calls](const HitEvent &) {
        ++calls;
    });
    bus.publish(HitEvent{10});
    bus.publish(HitEvent{20});
    cr_assert_eq(calls, 2);
}

Test(core_event_bus, other_event_types_are_not_received)
{
    EventBus bus;
    bool called = false;

    bus.subscribe<HitEvent>([&called](const HitEvent &) {
        called = true;
    });
    bus.publish(DeathEvent{100});
    cr_assert_not(called);
}

Test(core_event_bus, publish_without_subscribers_does_nothing)
{
    EventBus bus;

    cr_assert_no_throw(bus.publish(HitEvent{10}), std::exception);
}
