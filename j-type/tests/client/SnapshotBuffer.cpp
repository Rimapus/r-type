/*
** EPITECH PROJECT, 2026
** j-type/tests/client/SnapshotBuffer
** File description:
** Unit tests of the snapshots put back in order and interpolated
*/

#include <criterion/criterion.h>

#include <cstdint>
#include <optional>
#include <vector>

#include "jtype/common/EntityType.hpp"
#include "jtype/common/Messages.hpp"
#include "world/SnapshotBuffer.hpp"

using jtype::EntityState;
using jtype::EntityType;
using jtype::SnapshotBuffer;
using jtype::SnapshotMessage;

namespace {

// A snapshot of the server with one Bydo, the entity 1
SnapshotMessage oneBydo(std::uint32_t tick, float x)
{
    SnapshotMessage snapshot;

    snapshot.tick = tick;
    snapshot.entities.push_back(EntityState{1, EntityType::Blobling, x, 0.0F});
    return snapshot;
}

// The x of the only entity of the drawn world
float drawnX(SnapshotBuffer &buffer)
{
    const std::optional<SnapshotMessage> drawn = buffer.step();

    cr_assert(drawn.has_value());
    cr_assert_eq(drawn->entities.size(), 1);
    return drawn->entities[0].x;
}

// The ids of the entities of the drawn world
std::vector<std::uint32_t> drawnIds(SnapshotBuffer &buffer)
{
    const std::optional<SnapshotMessage> drawn = buffer.step();
    std::vector<std::uint32_t> ids;

    cr_assert(drawn.has_value());
    for (const EntityState &state : drawn->entities) {
        ids.push_back(state.id);
    }
    return ids;
}

} // namespace

Test(client_snapshot_buffer, draws_nothing_before_a_snapshot)
{
    SnapshotBuffer buffer;

    cr_assert_not(buffer.step().has_value());
}

Test(client_snapshot_buffer, draws_two_steps_behind_the_newest)
{
    SnapshotBuffer buffer;

    buffer.push(oneBydo(10, 10.0F));
    buffer.push(oneBydo(11, 11.0F));
    buffer.push(oneBydo(12, 12.0F));

    const float first = drawnX(buffer);
    const float second = drawnX(buffer);

    cr_assert_float_eq(first, 10.0F, 1e-6F);
    cr_assert_float_eq(second, 11.0F, 1e-6F);
}

Test(client_snapshot_buffer, fills_a_lost_snapshot)
{
    SnapshotBuffer buffer;

    // The snapshot 11 is lost: the drawn step 11 is halfway between 10 and 12
    buffer.push(oneBydo(10, 0.0F));
    buffer.push(oneBydo(12, 20.0F));
    buffer.push(oneBydo(13, 30.0F));

    const float x = drawnX(buffer);

    cr_assert_float_eq(x, 10.0F, 1e-6F);
}

Test(client_snapshot_buffer, puts_a_late_snapshot_back_in_order)
{
    SnapshotBuffer buffer;

    buffer.push(oneBydo(12, 12.0F));
    buffer.push(oneBydo(10, 10.0F));
    buffer.push(oneBydo(11, 11.0F));
    // A copy of a snapshot already kept changes nothing
    buffer.push(oneBydo(11, 99.0F));

    const float first = drawnX(buffer);
    const float second = drawnX(buffer);

    cr_assert_float_eq(first, 10.0F, 1e-6F);
    cr_assert_float_eq(second, 11.0F, 1e-6F);
}

Test(client_snapshot_buffer, holds_the_newest_when_the_snapshots_stop)
{
    SnapshotBuffer buffer;

    buffer.push(oneBydo(10, 10.0F));
    buffer.push(oneBydo(11, 11.0F));
    buffer.push(oneBydo(12, 12.0F));
    for (int step = 0; step < 3; ++step) {
        (void)buffer.step();
    }

    const float x = drawnX(buffer);

    cr_assert_float_eq(x, 12.0F, 1e-6F);
}

Test(client_snapshot_buffer, entity_comes_and_goes_with_its_snapshots)
{
    SnapshotBuffer buffer;
    SnapshotMessage shot = oneBydo(11, 1.0F);
    SnapshotMessage gone;

    // A missile appears at 11, and the Bydo is gone at 12
    shot.entities.push_back(EntityState{2, EntityType::Missile1, 5.0F, 0.0F});
    gone.tick = 12;
    gone.entities.push_back(EntityState{2, EntityType::Missile1, 6.0F, 0.0F});
    buffer.push(oneBydo(10, 0.0F));
    buffer.push(shot);
    buffer.push(gone);

    const std::vector<std::uint32_t> before = drawnIds(buffer);
    const std::vector<std::uint32_t> both = drawnIds(buffer);
    const std::vector<std::uint32_t> after = drawnIds(buffer);

    cr_assert(before == std::vector<std::uint32_t>{1});
    cr_assert(both == std::vector<std::uint32_t>({1, 2}));
    cr_assert(after == std::vector<std::uint32_t>{2});
}
