/*
** EPITECH PROJECT, 2026
** j-type/tests/client/SnapshotAssembler
** File description:
** Unit tests of the chunks of a snapshot put back together
*/

#include <criterion/criterion.h>

#include <cstddef>
#include <cstdint>
#include <optional>
#include <vector>

#include "jtype/common/EntityType.hpp"
#include "jtype/common/Messages.hpp"
#include "network/SnapshotAssembler.hpp"

using jtype::EntityState;
using jtype::EntityType;
using jtype::MAX_SNAPSHOT_ENTITIES;
using jtype::SnapshotAssembler;
using jtype::SnapshotMessage;
using jtype::splitSnapshot;

namespace {

// A snapshot of a step with entities numbered from 0
SnapshotMessage snapshotOf(std::uint32_t tick, std::size_t count)
{
    SnapshotMessage snapshot;

    snapshot.tick = tick;
    snapshot.entities.reserve(count);
    for (std::size_t index = 0; index < count; ++index) {
        snapshot.entities.push_back(EntityState{static_cast<std::uint32_t>(index), EntityType::Blobling,
                                                static_cast<float>(index), 0.0F, 1});
    }
    snapshot.entityCount = static_cast<std::uint16_t>(count);
    return snapshot;
}

} // namespace

Test(client_snapshot_assembler, single_chunk_passes_through)
{
    SnapshotAssembler assembler;
    const std::optional<SnapshotMessage> snapshot = assembler.push(snapshotOf(10, 1));

    cr_assert(snapshot.has_value());
    cr_assert_eq(snapshot->tick, 10);
    cr_assert_eq(snapshot->entities.size(), 1);
    cr_assert_eq(snapshot->entityCount, 1);
}

Test(client_snapshot_assembler, empty_snapshot_passes_through)
{
    SnapshotAssembler assembler;
    const std::optional<SnapshotMessage> snapshot = assembler.push(SnapshotMessage{});

    cr_assert(snapshot.has_value());
    cr_assert(snapshot->entities.empty());
    cr_assert_eq(snapshot->entityCount, 0);
}

Test(client_snapshot_assembler, chunks_are_put_back_in_order)
{
    SnapshotAssembler assembler;
    const std::size_t count = MAX_SNAPSHOT_ENTITIES + 5;
    std::vector<SnapshotMessage> chunks = splitSnapshot(snapshotOf(10, count));

    cr_assert_eq(chunks.size(), 2);
    cr_assert_not(assembler.push(std::move(chunks[1])).has_value());

    const std::optional<SnapshotMessage> snapshot = assembler.push(std::move(chunks[0]));

    cr_assert(snapshot.has_value());
    cr_assert_eq(snapshot->tick, 10);
    cr_assert_eq(snapshot->chunkIndex, 0);
    cr_assert_eq(snapshot->chunkCount, 1);
    cr_assert_eq(snapshot->entityCount, count);
    cr_assert_eq(snapshot->entities.size(), count);
    cr_assert_eq(snapshot->entities[0].id, 0);
    cr_assert_eq(snapshot->entities[count - 1].id, count - 1);
}

Test(client_snapshot_assembler, split_snapshot_keeps_its_last_input)
{
    SnapshotAssembler assembler;
    SnapshotMessage whole = snapshotOf(10, MAX_SNAPSHOT_ENTITIES + 1);

    whole.lastInput = 42;

    std::vector<SnapshotMessage> chunks = splitSnapshot(std::move(whole));

    cr_assert_eq(chunks[1].lastInput, 42);
    cr_assert_not(assembler.push(std::move(chunks[0])).has_value());

    const std::optional<SnapshotMessage> snapshot = assembler.push(std::move(chunks[1]));

    cr_assert(snapshot.has_value());
    cr_assert_eq(snapshot->lastInput, 42);
}

Test(client_snapshot_assembler, duplicated_chunk_is_ignored)
{
    SnapshotAssembler assembler;
    std::vector<SnapshotMessage> chunks = splitSnapshot(snapshotOf(10, MAX_SNAPSHOT_ENTITIES + 1));
    SnapshotMessage duplicate = chunks[0];

    cr_assert_not(assembler.push(std::move(chunks[0])).has_value());
    cr_assert_not(assembler.push(std::move(duplicate)).has_value());

    const std::optional<SnapshotMessage> snapshot = assembler.push(std::move(chunks[1]));

    cr_assert(snapshot.has_value());
    cr_assert_eq(snapshot->entities.size(), MAX_SNAPSHOT_ENTITIES + 1);
}

Test(client_snapshot_assembler, interleaved_snapshots_are_assembled_apart)
{
    SnapshotAssembler assembler;
    std::vector<SnapshotMessage> first = splitSnapshot(snapshotOf(10, MAX_SNAPSHOT_ENTITIES + 1));
    std::vector<SnapshotMessage> second = splitSnapshot(snapshotOf(11, MAX_SNAPSHOT_ENTITIES + 1));

    cr_assert_not(assembler.push(std::move(first[0])).has_value());
    cr_assert_not(assembler.push(std::move(second[0])).has_value());

    const std::optional<SnapshotMessage> doneFirst = assembler.push(std::move(first[1]));
    const std::optional<SnapshotMessage> doneSecond = assembler.push(std::move(second[1]));

    cr_assert(doneFirst.has_value());
    cr_assert_eq(doneFirst->tick, 10);
    cr_assert(doneSecond.has_value());
    cr_assert_eq(doneSecond->tick, 11);
}

Test(client_snapshot_assembler, invalid_chunk_is_refused)
{
    SnapshotAssembler assembler;
    SnapshotMessage chunk = snapshotOf(10, 1);

    chunk.chunkCount = 2;
    chunk.entityCount = 100;
    cr_assert_not(assembler.push(chunk).has_value());
}

Test(client_snapshot_assembler, chunks_that_disagree_drop_the_snapshot)
{
    SnapshotAssembler assembler;
    std::vector<SnapshotMessage> smaller = splitSnapshot(snapshotOf(10, MAX_SNAPSHOT_ENTITIES + 1));
    std::vector<SnapshotMessage> larger = splitSnapshot(snapshotOf(10, MAX_SNAPSHOT_ENTITIES + 2));

    cr_assert_not(assembler.push(std::move(smaller[0])).has_value());
    // The chunks of 11 entities claim another total than the first one: the step is dropped whole
    cr_assert_not(assembler.push(std::move(larger[0])).has_value());
    cr_assert_not(assembler.push(std::move(smaller[1])).has_value());
}
