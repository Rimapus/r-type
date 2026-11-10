/*
** EPITECH PROJECT, 2026
** j-type/tests/common/Messages
** File description:
** Unit tests of the network messages
*/

#include <criterion/criterion.h>

#include <cstddef>
#include <cstdint>
#include <expected>
#include <limits>
#include <span>
#include <variant>
#include <vector>

#include "jtype/common/Input.hpp"
#include "jtype/common/Messages.hpp"
#include "younity/network/Limits.hpp"
#include "younity/network/Packet.hpp"

using jtype::DeathEventMessage;
using jtype::DecodeError;
using jtype::decodeMessage;
using jtype::DestroyMessage;
using jtype::encodeMessage;
using jtype::EntityState;
using jtype::EntityType;
using jtype::FullMessage;
using jtype::GameOverEventMessage;
using jtype::getEventTick;
using jtype::InputMessage;
using jtype::isNewerTick;
using jtype::isValidChunk;
using jtype::JoinMessage;
using jtype::LeaveMessage;
using jtype::MAX_SNAPSHOT_CHUNKS;
using jtype::MAX_SNAPSHOT_ENTITIES;
using jtype::MAX_SNAPSHOT_TOTAL_ENTITIES;
using jtype::Message;
using jtype::PlayerDisconnectedMessage;
using jtype::PlayerStatus;
using jtype::ScoreboardMessage;
using jtype::ScoreEventMessage;
using jtype::SlotState;
using jtype::SnapshotMessage;
using jtype::SpawnMessage;
using jtype::splitSnapshot;
using jtype::toString;
using jtype::WelcomeMessage;
using younity::network::Packet;
using younity::network::PacketType;

namespace {

// Where the payload of a datagram starts, after the header of the engine
constexpr std::size_t PAYLOAD = younity::network::PACKET_HEADER_BYTES;

// Read a datagram as a peer does: the packet, then the message inside
std::expected<Message, DecodeError> receive(std::span<const std::byte> datagram)
{
    const auto packet = younity::network::decodePacket(datagram);

    cr_assert(packet.has_value());
    return decodeMessage(*packet);
}

// Encode a message, decode it back and check that its fields are unchanged
template <typename Content>
void roundtrip(const Content &message)
{
    const std::expected<Message, DecodeError> received = receive(encodeMessage(message, 1));

    cr_assert(received.has_value());
    cr_assert(std::holds_alternative<Content>(*received));
    cr_assert(std::get<Content>(*received) == message);
}

// Check that a datagram carries no message, for the reason expected
void expectError(std::span<const std::byte> datagram, DecodeError error)
{
    const std::expected<Message, DecodeError> received = receive(datagram);

    cr_assert_not(received.has_value());
    cr_assert(received.error() == error);
}

// A snapshot of entities of the same type, numbered from 0
SnapshotMessage snapshotOf(std::size_t count)
{
    SnapshotMessage snapshot;

    snapshot.tick = 4242;
    snapshot.entityCount = static_cast<std::uint16_t>(count);
    snapshot.entities.reserve(count);
    for (std::size_t index = 0; index < count; ++index) {
        snapshot.entities.push_back(EntityState{static_cast<std::uint32_t>(index), EntityType::Blobling,
                                                static_cast<float>(index), 0.0F, 1});
    }
    return snapshot;
}

void putU8(std::vector<std::byte> &datagram, std::size_t offset, std::uint8_t value)
{
    datagram[offset] = static_cast<std::byte>(value);
}

void putU32(std::vector<std::byte> &datagram, std::size_t offset, std::uint32_t value)
{
    for (std::size_t index = 0; index < sizeof(value); ++index) {
        datagram[offset + index] = static_cast<std::byte>((value >> (8 * index)) & 0xFF);
    }
}

} // namespace

Test(common_messages, client_messages_go_through_the_network)
{
    roundtrip(JoinMessage{});
    roundtrip(LeaveMessage{});
    roundtrip(WelcomeMessage{2});
    roundtrip(FullMessage{});
}

Test(common_messages, input_goes_through_the_network_with_its_tick)
{
    roundtrip(InputMessage{jtype::input::UP | jtype::input::SHOOT, 4242});
}

Test(common_messages, input_with_an_unknown_bit_is_rejected)
{
    std::vector<std::byte> datagram = encodeMessage(InputMessage{}, 1);

    // After the message type, the input bits
    putU8(datagram, PAYLOAD + 1, 0xFF);
    expectError(datagram, DecodeError::InvalidValue);
}

Test(common_messages, input_without_its_tick_is_truncated)
{
    std::vector<std::byte> datagram = encodeMessage(InputMessage{}, 1);

    datagram.pop_back();
    expectError(datagram, DecodeError::Truncated);
}

Test(common_messages, every_decode_error_has_a_description)
{
    cr_assert_eq(toString(DecodeError::NotGameData), "not a game message");
    cr_assert_eq(toString(DecodeError::Empty), "empty");
    cr_assert_eq(toString(DecodeError::UnknownType), "unknown type");
    cr_assert_eq(toString(DecodeError::Truncated), "truncated");
    cr_assert_eq(toString(DecodeError::InvalidValue), "invalid value");
    cr_assert_eq(toString(DecodeError::TrailingBytes), "trailing bytes");
    cr_assert_eq(toString(DecodeError::InvalidChunk), "invalid chunk");
}

Test(common_messages, session_packet_is_not_a_game_message)
{
    const Packet connect{PacketType::Connect, 0, {}};
    const std::expected<Message, DecodeError> received = decodeMessage(connect);

    cr_assert_not(received.has_value());
    cr_assert(received.error() == DecodeError::NotGameData);
}

Test(common_messages, empty_payload_is_rejected)
{
    cr_assert(decodeMessage(Packet{PacketType::GameData, 0, {}}).error() == DecodeError::Empty);
}

Test(common_messages, unknown_message_type_is_rejected)
{
    const std::vector<std::byte> payload{std::byte{0x7F}};

    cr_assert(decodeMessage(Packet{PacketType::GameData, 0, payload}).error() == DecodeError::UnknownType);
}

Test(common_messages, trailing_bytes_are_rejected)
{
    const std::vector<std::byte> payload{std::byte{1}, std::byte{0}};

    cr_assert(decodeMessage(Packet{PacketType::GameData, 0, payload}).error() == DecodeError::TrailingBytes);
}

Test(common_messages, snapshot_goes_through_the_network_with_its_tick_last_input_and_health)
{
    SnapshotMessage snapshot = snapshotOf(2);

    snapshot.tick = 4242;
    snapshot.lastInput = 77;
    snapshot.entities[1].type = EntityType::Jelly;
    snapshot.entities[1].x = 12.5F;
    snapshot.entities[1].y = 40.0F;
    snapshot.entities[1].health = 2;
    roundtrip(snapshot);
}

Test(common_messages, empty_snapshot_goes_through_the_network)
{
    // The client clears the world when the server sends no entity
    roundtrip(SnapshotMessage{});
}

Test(common_messages, events_give_their_tick)
{
    cr_assert(getEventTick(DestroyMessage{42, 7}) == 42U);
    cr_assert(getEventTick(GameOverEventMessage{43}) == 43U);
    cr_assert_not(getEventTick(WelcomeMessage{1}).has_value());
    cr_assert_not(getEventTick(snapshotOf(1)).has_value());
}

Test(common_messages, chunks_of_a_split_snapshot_are_valid)
{
    for (const SnapshotMessage &chunk : splitSnapshot(snapshotOf(MAX_SNAPSHOT_ENTITIES + 1))) {
        cr_assert(isValidChunk(chunk, chunk.entities.size()));
    }
}

Test(common_messages, chunk_with_a_wrong_layout_is_invalid)
{
    SnapshotMessage chunk = splitSnapshot(snapshotOf(MAX_SNAPSHOT_ENTITIES + 1)).front();

    cr_assert_not(isValidChunk(chunk, chunk.entities.size() - 1));
    chunk.chunkIndex = chunk.chunkCount;
    cr_assert_not(isValidChunk(chunk, chunk.entities.size()));
    chunk.chunkIndex = 0;
    chunk.chunkCount = 1;
    cr_assert_not(isValidChunk(chunk, chunk.entities.size()));
}

Test(common_messages, snapshot_is_split_into_datagram_sized_chunks)
{
    const std::vector<SnapshotMessage> chunks = splitSnapshot(snapshotOf(MAX_SNAPSHOT_ENTITIES + 1));

    cr_assert_eq(chunks.size(), 2);
    cr_assert_eq(chunks[0].chunkIndex, 0);
    cr_assert_eq(chunks[1].chunkIndex, 1);
    cr_assert_eq(chunks[0].chunkCount, 2);
    cr_assert_eq(chunks[0].entityCount, MAX_SNAPSHOT_ENTITIES + 1);
    cr_assert_eq(chunks[0].entities.size(), MAX_SNAPSHOT_ENTITIES);
    cr_assert_eq(chunks[1].entities.size(), 1);
    roundtrip(chunks[0]);
    roundtrip(chunks[1]);
}

Test(common_messages, snapshot_takes_the_maximum_number_of_chunks)
{
    const std::vector<SnapshotMessage> chunks = splitSnapshot(snapshotOf(MAX_SNAPSHOT_TOTAL_ENTITIES));

    cr_assert_eq(chunks.size(), MAX_SNAPSHOT_CHUNKS);
    cr_assert_eq(chunks.back().entities.size(), MAX_SNAPSHOT_ENTITIES);
    roundtrip(chunks.back());
}

Test(common_messages, snapshot_over_the_maximum_is_refused)
{
    const SnapshotMessage snapshot = snapshotOf(MAX_SNAPSHOT_TOTAL_ENTITIES + 1);

    cr_assert(splitSnapshot(snapshot).empty());
    cr_assert(encodeMessage(snapshot, 1).empty());
}

Test(common_messages, chunk_count_must_match_the_total)
{
    std::vector<SnapshotMessage> chunks = splitSnapshot(snapshotOf(MAX_SNAPSHOT_ENTITIES + 1));
    std::vector<std::byte> datagram = encodeMessage(chunks[0], 1);

    // After the message type, the tick, the last input, the chunk index: the number of chunks
    putU8(datagram, PAYLOAD + 1 + 4 + 4 + 1, 3);
    expectError(datagram, DecodeError::InvalidChunk);
}

Test(common_messages, scoreboard_goes_through_the_network)
{
    ScoreboardMessage scoreboard;

    scoreboard.tick = 4242;
    scoreboard.players[0] = PlayerStatus{SlotState::Playing, 1200, 2};
    scoreboard.players[2] = PlayerStatus{SlotState::Out, 300, 0};
    scoreboard.players[3] = PlayerStatus{SlotState::Left, 50, 0};
    scoreboard.players[0].weapon = jtype::Weapon::Spread;
    scoreboard.gameOver = true;
    roundtrip(scoreboard);
}

Test(common_messages, victory_goes_through_the_network)
{
    ScoreboardMessage scoreboard;

    scoreboard.gameOver = true;
    scoreboard.victory = true;
    roundtrip(scoreboard);
}

Test(common_messages, boss_goes_through_the_network)
{
    for (const EntityType type : {EntityType::IncubatorBody, EntityType::IncubatorEye, EntityType::IncubatorEyeAngry,
                                  EntityType::IncubatorOrb, EntityType::IncubatorSpit, EntityType::IncubatorWreck}) {
        roundtrip(SpawnMessage{
            4242, EntityState{7, type, 1.0F, 2.0F, 3}
        });
    }
}

Test(common_messages, scoreboard_with_an_unknown_weapon_is_rejected)
{
    std::vector<std::byte> datagram = encodeMessage(ScoreboardMessage{}, 1);

    // After the message type, the tick, the end of the game, then the state, the score and the lives of the first slot
    putU8(datagram, PAYLOAD + 1 + 4 + 1 + 1 + 4 + 1, 2);
    expectError(datagram, DecodeError::InvalidValue);
}

Test(common_messages, pick_up_goes_through_the_network)
{
    roundtrip(jtype::PickUpEventMessage{4242, 3, jtype::PowerUpType::SpreadShot});
    for (const EntityType type : {EntityType::SpreadPellet1, EntityType::SpreadPellet4, EntityType::ExtraLifePowerUp,
                                  EntityType::ShieldPowerUp, EntityType::SpreadShotPowerUp}) {
        roundtrip(SpawnMessage{
            4242, EntityState{7, type, 1.0F, 2.0F, 0}
        });
    }
}

Test(common_messages, obstacles_go_through_the_network)
{
    for (const EntityType type : {EntityType::Block, EntityType::CrackedBlock, EntityType::ZapperNode, EntityType::ZapperBeam}) {
        roundtrip(SpawnMessage{
            4242, EntityState{7, type, 1.0F, 2.0F, 0}
        });
    }
}

Test(common_messages, pick_up_with_an_unknown_power_up_is_rejected)
{
    std::vector<std::byte> datagram = encodeMessage(jtype::PickUpEventMessage{}, 1);

    // After the message type, the tick and the slot: the power-up
    putU8(datagram, PAYLOAD + 1 + 4 + 1, 3);
    expectError(datagram, DecodeError::InvalidValue);
}

Test(common_messages, truncated_scoreboard_is_rejected)
{
    std::vector<std::byte> datagram = encodeMessage(ScoreboardMessage{}, 1);

    datagram.pop_back();
    expectError(datagram, DecodeError::Truncated);
}

Test(common_messages, scoreboard_with_an_unknown_slot_state_is_rejected)
{
    std::vector<std::byte> datagram = encodeMessage(ScoreboardMessage{}, 1);

    // After the message type, the tick and the game over: the state of the first slot
    putU8(datagram, PAYLOAD + 1 + 4 + 1, 4);
    expectError(datagram, DecodeError::InvalidValue);
}

Test(common_messages, scoreboard_with_an_unknown_end_is_rejected)
{
    std::vector<std::byte> datagram = encodeMessage(ScoreboardMessage{}, 1);

    // After the message type and the tick: the end of the game, 0 to 2
    putU8(datagram, PAYLOAD + 1 + 4, 3);
    expectError(datagram, DecodeError::InvalidValue);
}

Test(common_messages, events_go_through_the_network)
{
    roundtrip(SpawnMessage{
        4242, EntityState{7, EntityType::SentryDrone, 1.0F, 2.0F, 3}
    });
    roundtrip(DestroyMessage{4242, 7});
    roundtrip(PlayerDisconnectedMessage{4242, 3});
    roundtrip(ScoreEventMessage{4242, 1, 200});
    roundtrip(DeathEventMessage{4242, 7, jtype::NO_PLAYER});
    roundtrip(DeathEventMessage{4242, 7, 2});
    roundtrip(GameOverEventMessage{4242});
}

Test(common_messages, event_with_an_unknown_killer_is_rejected)
{
    std::vector<std::byte> datagram = encodeMessage(DeathEventMessage{4242, 7, 2}, 1);

    // After the message type, the tick and the id: the slot of the killer
    putU8(datagram, PAYLOAD + 1 + 4 + 4, 9);
    expectError(datagram, DecodeError::InvalidValue);
}

Test(common_messages, event_with_an_unknown_slot_is_rejected)
{
    std::vector<std::byte> datagram = encodeMessage(PlayerDisconnectedMessage{4242, 1}, 1);

    // After the message type and the tick: the slot
    putU8(datagram, PAYLOAD + 1 + 4, 4);
    expectError(datagram, DecodeError::InvalidValue);
}

Test(common_messages, entity_with_an_unknown_type_is_rejected)
{
    std::vector<std::byte> datagram = encodeMessage(SpawnMessage{}, 1);

    // After the message type, the tick and the id: the type of the entity, past the last one
    putU8(datagram, PAYLOAD + 1 + 4 + 4, static_cast<std::uint8_t>(EntityType::ZapperBeam) + 1);
    expectError(datagram, DecodeError::InvalidValue);
}

Test(common_messages, entity_with_a_non_finite_position_is_rejected)
{
    std::vector<std::byte> datagram = encodeMessage(SpawnMessage{}, 1);

    // After the message type, the tick, the id and the type: the x of the entity
    putU32(datagram, PAYLOAD + 1 + 4 + 4 + 1, 0x7FC00000);
    expectError(datagram, DecodeError::InvalidValue);
}

Test(common_messages, welcome_with_an_unknown_slot_is_rejected)
{
    std::vector<std::byte> datagram = encodeMessage(WelcomeMessage{1}, 1);

    putU8(datagram, PAYLOAD + 1, 4);
    expectError(datagram, DecodeError::InvalidValue);
}

Test(common_messages, tick_comparison_wraps_around)
{
    cr_assert(isNewerTick(1, 0));
    cr_assert_not(isNewerTick(0, 1));
    cr_assert_not(isNewerTick(5, 5));
    cr_assert(isNewerTick(0, std::numeric_limits<std::uint32_t>::max()));
    cr_assert_not(isNewerTick(std::numeric_limits<std::uint32_t>::max(), 0));
}

Test(common_messages, a_full_snapshot_chunk_fits_in_a_datagram)
{
    cr_assert_eq(MAX_SNAPSHOT_ENTITIES, 84);
    cr_assert_eq(MAX_SNAPSHOT_CHUNKS, 64);
    cr_assert_eq(MAX_SNAPSHOT_TOTAL_ENTITIES, 5376);
    cr_assert_leq(PAYLOAD + jtype::SNAPSHOT_HEADER_BYTES + MAX_SNAPSHOT_ENTITIES * jtype::ENTITY_STATE_BYTES,
                  younity::network::MAX_PACKET_BYTES);
}
