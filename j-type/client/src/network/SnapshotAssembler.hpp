/*
** EPITECH PROJECT, 2026
** j-type/client/src/network/SnapshotAssembler
** File description:
** SnapshotAssembler header file
*/

#pragma once

#include <cstdint>
#include <deque>
#include <optional>
#include <vector>

#include "jtype/common/Messages.hpp"

namespace jtype {

/**
 * @brief Puts the chunks of the snapshots of the server back together before the SnapshotBuffer sees them.
 *        Chunks of several steps may arrive interleaved or out of order: each step is assembled on its own.
 */
class SnapshotAssembler
{
public:
    /**
     * @brief Add a chunk received, and get the snapshot once its last chunk is there.
     *
     * @param chunk The chunk received, whatever its place among the chunks of its snapshot
     *
     * @return The whole snapshot when this chunk completed it, or nothing while chunks are missing or if the chunk is
     *         invalid or a duplicate.
     */
    [[nodiscard]] std::optional<SnapshotMessage> push(SnapshotMessage chunk);

private:
    // A snapshot being assembled, with its chunks received so far
    struct Pending
    {
        std::uint32_t tick = 0;
        std::uint8_t chunkCount = 1;
        std::uint16_t entityCount = 0;
        std::vector<std::optional<SnapshotMessage>> chunks;
        std::uint8_t missing = 0;
    };

    // The snapshots being assembled, the oldest first; only the newest steps are worth waiting for
    std::deque<Pending> _pending;
};

} // namespace jtype
