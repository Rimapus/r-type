/*
** EPITECH PROJECT, 2026
** j-type/client/src/world/SnapshotBuffer
** File description:
** SnapshotBuffer header file
*/

#pragma once

#include <cstdint>
#include <deque>
#include <optional>

#include "jtype/common/Messages.hpp"

namespace jtype {

/**
 * @brief The snapshots received, in the order of the server, and the world drawn a few steps behind the newest one:
 *        between the two snapshots around it, so a late or a lost snapshot does not show.
 */
class SnapshotBuffer
{
public:
    /**
     * @brief Keep a snapshot in its place among the others, whatever the order it came in. A snapshot already kept is
     *        dropped.
     *
     * @param snapshot The snapshot received
     */
    void push(SnapshotMessage snapshot);

    /**
     * @brief Move the drawn world one step forward and get it. It stays about INTERPOLATION_DELAY steps behind the
     *        newest snapshot, and holds the newest one when the snapshots stop coming.
     *
     * @return The entities of the snapshot before the drawn step, each moved toward its position in the snapshot
     *         after; or nothing before the first snapshot.
     */
    [[nodiscard]] std::optional<SnapshotMessage> step(void);

private:
    // Sorted by tick, from the one before the drawn step
    std::deque<SnapshotMessage> _snapshots;
    // The step of the server drawn, empty before the first snapshot
    std::optional<std::int64_t> _drawn;
};

} // namespace jtype
