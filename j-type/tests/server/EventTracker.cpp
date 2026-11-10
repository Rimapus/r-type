/*
** EPITECH PROJECT, 2026
** j-type/tests/server/EventTracker
** File description:
** Unit tests of the events found from the changes of a step
*/

#include <criterion/criterion.h>

#include <cstdint>
#include <variant>
#include <vector>

#include "EventTracker.hpp"
#include "jtype/common/Messages.hpp"

using jtype::DestroyMessage;
using jtype::EntityState;
using jtype::EntityType;
using jtype::EventTracker;
using jtype::GameOverEventMessage;
using jtype::Message;
using jtype::ScoreboardMessage;
using jtype::ScoreEventMessage;
using jtype::SpawnMessage;

namespace {

EntityState entityOf(std::uint32_t id, EntityType type = EntityType::Blobling)
{
    return EntityState{id, type, 1.0F, 2.0F, 1};
}

// Run one step of the tracker with a fixed number, so the tests check the events, not the clock
std::vector<Message> update(EventTracker &tracker, const std::vector<EntityState> &entities,
                            const ScoreboardMessage &scoreboard = {})
{
    return tracker.update(42, entities, scoreboard);
}

} // namespace

Test(server_event_tracker, first_step_spawns_every_entity)
{
    EventTracker tracker;
    const std::vector<Message> events = update(tracker, {entityOf(1), entityOf(2)});

    cr_assert_eq(events.size(), 2U);
    cr_assert(std::holds_alternative<SpawnMessage>(events[0]));
    cr_assert(std::holds_alternative<SpawnMessage>(events[1]));
    cr_assert_eq(std::get<SpawnMessage>(events[0]).tick, 42);
}

Test(server_event_tracker, a_moved_entity_raises_no_event)
{
    EventTracker tracker;
    std::vector<EntityState> entities = {entityOf(1)};

    (void)update(tracker, entities);
    entities[0].x = 99.0F;
    cr_assert(update(tracker, entities).empty());
}

Test(server_event_tracker, an_entity_that_goes_is_destroyed)
{
    EventTracker tracker;

    (void)update(tracker, {entityOf(1), entityOf(2)});
    const std::vector<Message> events = update(tracker, {entityOf(2)});

    cr_assert_eq(events.size(), 1U);
    cr_assert(std::holds_alternative<DestroyMessage>(events[0]));
    cr_assert_eq(std::get<DestroyMessage>(events[0]).tick, 42);
    cr_assert_eq(std::get<DestroyMessage>(events[0]).id, 1);
}

Test(server_event_tracker, an_entity_that_appears_is_spawned)
{
    EventTracker tracker;

    (void)update(tracker, {entityOf(1)});
    const std::vector<Message> events = update(tracker, {entityOf(1), entityOf(2, EntityType::Jelly)});

    cr_assert_eq(events.size(), 1U);
    cr_assert(std::holds_alternative<SpawnMessage>(events[0]));
    cr_assert(std::get<SpawnMessage>(events[0]).entity.id == 2);
    cr_assert(std::get<SpawnMessage>(events[0]).entity.type == EntityType::Jelly);
}

Test(server_event_tracker, an_id_given_to_another_type_is_destroyed_then_spawned)
{
    EventTracker tracker;

    (void)update(tracker, {entityOf(7)});
    const std::vector<Message> events = update(tracker, {entityOf(7, EntityType::Missile1)});

    cr_assert_eq(events.size(), 2U);
    cr_assert(std::holds_alternative<DestroyMessage>(events[0]));
    cr_assert_eq(std::get<DestroyMessage>(events[0]).id, 7);
    cr_assert(std::holds_alternative<SpawnMessage>(events[1]));
    cr_assert_eq(std::get<SpawnMessage>(events[1]).entity.id, 7);
    cr_assert(std::get<SpawnMessage>(events[1]).entity.type == EntityType::Missile1);
}

Test(server_event_tracker, an_empty_step_destroys_every_entity)
{
    EventTracker tracker;

    (void)update(tracker, {entityOf(1), entityOf(2)});
    const std::vector<Message> events = update(tracker, {});

    cr_assert_eq(events.size(), 2U);
    cr_assert(std::holds_alternative<DestroyMessage>(events[0]));
    cr_assert(std::holds_alternative<DestroyMessage>(events[1]));
}

Test(server_event_tracker, a_score_increase_raises_its_event_once)
{
    EventTracker tracker;
    ScoreboardMessage scoreboard;

    scoreboard.players[1].score = 150;
    const std::vector<Message> events = update(tracker, {}, scoreboard);

    cr_assert_eq(events.size(), 1U);
    cr_assert(std::holds_alternative<ScoreEventMessage>(events[0]));
    cr_assert_eq(std::get<ScoreEventMessage>(events[0]).slot, 1);
    cr_assert_eq(std::get<ScoreEventMessage>(events[0]).points, 150);
    cr_assert(update(tracker, {}, scoreboard).empty());

    scoreboard.players[1].score = 400;
    const std::vector<Message> after = update(tracker, {}, scoreboard);

    cr_assert_eq(after.size(), 1U);
    cr_assert_eq(std::get<ScoreEventMessage>(after[0]).points, 250);
}

Test(server_event_tracker, a_new_game_raises_no_score_event)
{
    EventTracker tracker;
    ScoreboardMessage scoreboard;

    scoreboard.players[0].score = 300;
    (void)update(tracker, {}, scoreboard);
    scoreboard.players[0].score = 0;
    cr_assert(update(tracker, {}, scoreboard).empty());
}

Test(server_event_tracker, the_game_over_is_raised_once)
{
    EventTracker tracker;
    ScoreboardMessage scoreboard;

    cr_assert(update(tracker, {}, scoreboard).empty());
    scoreboard.gameOver = true;
    const std::vector<Message> events = update(tracker, {}, scoreboard);

    cr_assert_eq(events.size(), 1U);
    cr_assert(std::holds_alternative<GameOverEventMessage>(events[0]));
    cr_assert_eq(std::get<GameOverEventMessage>(events[0]).tick, 42);
    cr_assert(update(tracker, {}, scoreboard).empty());
}
