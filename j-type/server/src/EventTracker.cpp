/*
** EPITECH PROJECT, 2026
** j-type/server/src/EventTracker
** File description:
** EventTracker source file
*/

#include "EventTracker.hpp"

#include <cstddef>
#include <cstdint>
#include <unordered_map>
#include <utility>
#include <vector>

namespace jtype {

std::vector<Message> EventTracker::update(std::uint32_t tick, const std::vector<EntityState> &entities,
                                          const ScoreboardMessage &scoreboard)
{
    std::vector<Message> events;
    std::unordered_map<std::uint32_t, EntityType> current;

    current.reserve(entities.size());
    for (const EntityState &entity : entities) {
        current.emplace(entity.id, entity.type);
    }
    for (const auto &[id, type] : this->_entities) {
        const auto found = current.find(id);

        // An id given to another type is another entity: the old one went, the new one appeared
        if (found == current.end() || found->second != type) {
            events.emplace_back(DestroyMessage{tick, id});
        }
    }
    for (const EntityState &entity : entities) {
        const auto found = this->_entities.find(entity.id);

        if (found == this->_entities.end() || found->second != entity.type) {
            events.emplace_back(SpawnMessage{tick, entity});
        }
    }
    this->_entities = std::move(current);
    for (std::size_t slot = 0; slot < MAX_PLAYERS; ++slot) {
        const std::uint32_t score = scoreboard.players[slot].score;

        if (score > this->_scores[slot]) {
            events.emplace_back(ScoreEventMessage{tick, static_cast<std::uint8_t>(slot), score - this->_scores[slot]});
        }
        this->_scores[slot] = score;
    }
    if (scoreboard.gameOver && !this->_gameOver) {
        events.emplace_back(GameOverEventMessage{tick});
    }
    this->_gameOver = scoreboard.gameOver;
    return events;
}

} // namespace jtype
