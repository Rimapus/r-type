/*
** EPITECH PROJECT, 2026
** j-type/client/src/world/SnapshotBuffer
** File description:
** SnapshotBuffer source file
*/

#include "world/SnapshotBuffer.hpp"

#include <algorithm>
#include <iterator>
#include <unordered_map>
#include <utility>

namespace jtype {

namespace {

// The steps the drawn world stays behind the newest snapshot: enough to fill a snapshot lost or late by one step
constexpr std::int64_t INTERPOLATION_DELAY = 2;

/**
 * @brief Get the world between two snapshots.
 *
 * @param before The snapshot at or before the drawn step
 * @param after The snapshot after the drawn step
 * @param tick The drawn step
 *
 * @return The entities of before, each moved toward its position in after.
 */
SnapshotMessage interpolate(const SnapshotMessage &before, const SnapshotMessage &after, std::int64_t tick)
{
    const float progress = static_cast<float>(tick - before.tick) / static_cast<float>(after.tick - before.tick);
    std::unordered_map<std::uint32_t, const EntityState *> next;
    SnapshotMessage world = before;

    for (const EntityState &state : after.entities) {
        next.emplace(state.id, &state);
    }
    for (EntityState &state : world.entities) {
        const auto found = next.find(state.id);

        // An entity gone from after stays where it was, and an id reused by another type is another entity
        if (found != next.end() && found->second->type == state.type) {
            state.x += (found->second->x - state.x) * progress;
            state.y += (found->second->y - state.y) * progress;
        }
    }
    return world;
}

} // namespace

void SnapshotBuffer::push(SnapshotMessage snapshot)
{
    const auto place = std::ranges::lower_bound(this->_snapshots, snapshot.tick, {}, &SnapshotMessage::tick);

    if (place != this->_snapshots.end() && place->tick == snapshot.tick) {
        return;
    }
    this->_snapshots.insert(place, std::move(snapshot));
}

std::optional<SnapshotMessage> SnapshotBuffer::step(void)
{
    if (this->_snapshots.empty()) {
        return std::nullopt;
    }
    const std::int64_t newest = this->_snapshots.back().tick;

    // One step drawn per step run: the snapshots coming in bursts, late or not at all only bound it
    this->_drawn = this->_drawn ? std::clamp(*this->_drawn + 1, newest - (2 * INTERPOLATION_DELAY), newest)
                                : newest - INTERPOLATION_DELAY;
    const auto after = std::ranges::upper_bound(this->_snapshots, *this->_drawn, {}, &SnapshotMessage::tick);

    if (after == this->_snapshots.begin()) {
        return this->_snapshots.front();
    }
    // The snapshots before the one at the drawn step are never drawn again
    this->_snapshots.erase(this->_snapshots.begin(), std::prev(after));
    if (this->_snapshots.size() == 1) {
        return this->_snapshots.front();
    }
    return interpolate(this->_snapshots[0], this->_snapshots[1], *this->_drawn);
}

} // namespace jtype
