/*
** EPITECH PROJECT, 2026
** j-type/client/src/network/SnapshotAssembler
** File description:
** SnapshotAssembler source file
*/

#include "network/SnapshotAssembler.hpp"

#include <algorithm>
#include <cstddef>
#include <iterator>
#include <utility>

namespace jtype {

namespace {

// The snapshots being assembled at once: a snapshot is sent at each step, so only the newest ones matter
constexpr std::size_t MAX_PENDING = 4;

} // namespace

std::optional<SnapshotMessage> SnapshotAssembler::push(SnapshotMessage chunk)
{
    if (!isValidChunk(chunk, chunk.entities.size())) {
        return std::nullopt;
    }
    if (chunk.chunkCount == 1) {
        return chunk;
    }
    auto pending = std::ranges::find(this->_pending, chunk.tick, &Pending::tick);

    if (pending == this->_pending.end()) {
        if (this->_pending.size() == MAX_PENDING) {
            this->_pending.pop_front();
        }
        Pending created;

        created.tick = chunk.tick;
        created.chunkCount = chunk.chunkCount;
        created.entityCount = chunk.entityCount;
        created.chunks.resize(chunk.chunkCount);
        created.missing = chunk.chunkCount;
        this->_pending.push_back(std::move(created));
        pending = std::prev(this->_pending.end());
    } else if (pending->chunkCount != chunk.chunkCount || pending->entityCount != chunk.entityCount) {
        // The chunks of a step disagree on the snapshot they belong to: drop all of them
        this->_pending.erase(pending);
        return std::nullopt;
    }
    std::optional<SnapshotMessage> &slot = pending->chunks[chunk.chunkIndex];

    if (slot) {
        return std::nullopt;
    }
    slot = std::move(chunk);
    if (--pending->missing != 0) {
        return std::nullopt;
    }
    SnapshotMessage snapshot;

    snapshot.tick = pending->tick;
    // The server sends every chunk of a step to a client with the same last input
    snapshot.lastInput = pending->chunks.front()->lastInput;
    snapshot.entityCount = pending->entityCount;
    snapshot.entities.reserve(snapshot.entityCount);
    for (std::optional<SnapshotMessage> &part : pending->chunks) {
        snapshot.entities.insert(snapshot.entities.end(), std::make_move_iterator(part->entities.begin()),
                                 std::make_move_iterator(part->entities.end()));
    }
    this->_pending.erase(pending);
    return snapshot;
}

} // namespace jtype
