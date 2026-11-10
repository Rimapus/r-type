/*
** EPITECH PROJECT, 2026
** j-type/server/src/EventTracker
** File description:
** EventTracker header file
*/

#pragma once

#include <array>
#include <cstdint>
#include <unordered_map>
#include <vector>

#include "jtype/common/Constants.hpp"
#include "jtype/common/EntityType.hpp"
#include "jtype/common/Messages.hpp"

namespace jtype {

/**
 * @brief Find the events of a step from what changed since the previous one: the entities that appeared or went, the
 *        points each player earned and the game over.
 *        Every event repeats a change the next snapshot or scoreboard already carries, so a lost one costs no state.
 */
class EventTracker
{
public:
    /**
     * @brief Compare a step to the previous one.
     *
     * @param tick The step of the server
     * @param entities The entities of its snapshot
     * @param scoreboard Its scoreboard
     *
     * @return The events of the step, in a fixed order: the destructions, the spawns, the scores and the game over.
     */
    [[nodiscard]] std::vector<Message> update(std::uint32_t tick, const std::vector<EntityState> &entities,
                                              const ScoreboardMessage &scoreboard);

private:
    // The type of each entity of the previous step, to notice the ones that appeared, went or changed
    std::unordered_map<std::uint32_t, EntityType> _entities;
    std::array<std::uint32_t, MAX_PLAYERS> _scores{};
    bool _gameOver = false;
};

} // namespace jtype
