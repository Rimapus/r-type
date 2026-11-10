/*
** EPITECH PROJECT, 2026
** j-type/common/include/jtype/common/Messages
** File description:
** Messages header file
*/

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <optional>
#include <string_view>
#include <variant>
#include <vector>

#include "jtype/common/Constants.hpp"
#include "jtype/common/EntityType.hpp"
#include "jtype/common/PowerUpType.hpp"
#include "jtype/common/Weapon.hpp"
#include "younity/network/Limits.hpp"
#include "younity/network/Packet.hpp"

namespace jtype {

/**
 * @brief The first byte of the payload of a GameData packet, telling which message follows.
 *        A value is a byte of the protocol: a new one is added at the end, so the others keep their meaning.
 */
enum class MessageType : std::uint8_t {
    Join = 1,
    Welcome = 2,
    Input = 3,
    Snapshot = 4,
    Scoreboard = 5,
    Leave = 6,
    Full = 7,
    Spawn = 8,
    Destroy = 9,
    PlayerDisconnected = 10,
    ScoreEvent = 11,
    DeathEvent = 12,
    GameOverEvent = 13,
    PickUpEvent = 14
};

/**
 * @brief Why decodeMessage() rejected a packet.
 */
enum class DecodeError : std::uint8_t {
    // A session packet of the engine, not a game message
    NotGameData,
    // The payload does not even hold a message id
    Empty,
    // The message id is not one of MessageType
    UnknownType,
    // A field ends after the payload
    Truncated,
    // A field is outside of its range, e.g. an unknown entity type
    InvalidValue,
    // The payload goes on after the end of the message
    TrailingBytes,
    // The parts of a split snapshot do not fit together
    InvalidChunk
};

/**
 * @brief Get a short human-readable description of a decode error.
 *
 * @param error The error to describe
 *
 * @return The description, e.g. "invalid value".
 */
[[nodiscard]] std::string_view toString(DecodeError error) noexcept;

/**
 * @brief Client to server, once its session is open: asks for a slot in the game.
 */
struct JoinMessage
{
    bool operator==(const JoinMessage &other) const = default;
};

/**
 * @brief Client to server: the player quits, so its slot is freed without waiting for the session timeout.
 */
struct LeaveMessage
{
    bool operator==(const LeaveMessage &other) const = default;
};

/**
 * @brief Server to client: the slot given to the client.
 */
struct WelcomeMessage
{
    std::uint8_t slot = 0;

    bool operator==(const WelcomeMessage &other) const = default;
};

/**
 * @brief Server to client, instead of Welcome: the four slots are taken, so the client gets none.
 */
struct FullMessage
{
    bool operator==(const FullMessage &other) const = default;
};

/**
 * @brief Client to server: the actions held down and the step they were read at.
 */
struct InputMessage
{
    // The bits of input::UP to input::SHOOT, see Input.hpp
    std::uint8_t inputs = 0;
    // The step of the client, from 1: the server drops an input that is not newer than the last one, and gives the
    // last one back in the snapshots
    std::uint32_t tick = 0;

    bool operator==(const InputMessage &other) const = default;
};

/**
 * @brief One entity of a snapshot, and of a Spawn event.
 */
struct EntityState
{
    // The id of the entity in the registry of the server: both sides name the entity by it
    std::uint32_t id = 0;
    EntityType type = EntityType::Blobling;
    float x = 0.0F;
    float y = 0.0F;
    // The health points left, or 0 for an entity without health, e.g. a missile
    std::uint8_t health = 0;

    bool operator==(const EntityState &other) const = default;
};

/**
 * @brief Server to each client: one part of the entities of the game at a step, and the last input of this client.
 *        A snapshot is split in chunks when it does not fit in one datagram, see splitSnapshot().
 */
struct SnapshotMessage
{
    // The step of the server it was taken at, so the client puts the snapshots back in order
    std::uint32_t tick = 0;
    // The tick of the last input of this client, which its ship moved with: the client predicts from there
    std::uint32_t lastInput = 0;
    // The part of the snapshot this datagram carries, from 0 to chunkCount - 1
    std::uint8_t chunkIndex = 0;
    // The number of datagrams the snapshot is split into, at least 1
    std::uint8_t chunkCount = 1;
    // The entities of the whole snapshot, not only of this part
    std::uint16_t entityCount = 0;
    // The entities of this part
    std::vector<EntityState> entities;

    bool operator==(const SnapshotMessage &other) const = default;
};

/**
 * @brief Server to every client: an entity appeared, at the step of the event.
 */
struct SpawnMessage
{
    std::uint32_t tick = 0;
    EntityState entity;

    bool operator==(const SpawnMessage &other) const = default;
};

/**
 * @brief Server to every client: an entity is gone, at the step of the event.
 */
struct DestroyMessage
{
    std::uint32_t tick = 0;
    std::uint32_t id = 0;

    bool operator==(const DestroyMessage &other) const = default;
};

/**
 * @brief Server to every client: the player of a slot left, at the step of the event.
 */
struct PlayerDisconnectedMessage
{
    std::uint32_t tick = 0;
    std::uint8_t slot = 0;

    bool operator==(const PlayerDisconnectedMessage &other) const = default;
};

/**
 * @brief Server to every client: a player earned points, at the step of the event.
 */
struct ScoreEventMessage
{
    std::uint32_t tick = 0;
    std::uint8_t slot = 0;
    // The points earned, already counted in the next scoreboard
    std::uint32_t points = 0;

    bool operator==(const ScoreEventMessage &other) const = default;
};

/**
 * @brief Server to every client: a ship or a Bydo was destroyed, at the step of the event.
 */
struct DeathEventMessage
{
    std::uint32_t tick = 0;
    // The id of the entity that was destroyed
    std::uint32_t id = 0;
    // The slot of the player who destroyed it, NO_PLAYER for a crash or a Bydo missile
    std::uint8_t killer = NO_PLAYER;

    bool operator==(const DeathEventMessage &other) const = default;
};

/**
 * @brief Server to every client: the game is over, at the step of the event.
 */
struct GameOverEventMessage
{
    std::uint32_t tick = 0;

    bool operator==(const GameOverEventMessage &other) const = default;
};

/**
 * @brief Server to every client: the ship of a player took a power-up, at the step of the event.
 */
struct PickUpEventMessage
{
    std::uint32_t tick = 0;
    // The slot of the player whose ship took it
    std::uint8_t slot = 0;
    PowerUpType powerUp = PowerUpType::ExtraLife;

    bool operator==(const PickUpEventMessage &other) const = default;
};

/**
 * @brief Where the player of a slot is in the game, a byte of the scoreboard.
 */
enum class SlotState : std::uint8_t {
    // Nobody joined on this slot
    Free = 0,
    // The player has a ship
    Playing = 1,
    // The player is out of lives, until the next game starts
    Out = 2,
    // The player quit or its session timed out, until the next game starts or someone takes the slot
    Left = 3
};

/**
 * @brief How a player of a slot is doing, for the HUD.
 */
struct PlayerStatus
{
    SlotState state = SlotState::Free;
    std::uint32_t score = 0;
    std::uint8_t lives = 0;
    Weapon weapon = Weapon::Plasma;

    bool operator==(const PlayerStatus &other) const = default;
};

/**
 * @brief Server to every client, with each snapshot: the score, the lives and the weapon of each slot, and the end of
 *        the game.
 */
struct ScoreboardMessage
{
    // The step of the scoreboard, compared to the one of the snapshots and of the events
    std::uint32_t tick = 0;
    std::array<PlayerStatus, MAX_PLAYERS> players{};
    // true once the game is over, until the next game starts: every player is out of lives, or the boss is beaten
    bool gameOver = false;
    // true with gameOver when the boss is beaten
    bool victory = false;

    bool operator==(const ScoreboardMessage &other) const = default;
};

using Message = std::variant<JoinMessage, LeaveMessage, WelcomeMessage, FullMessage, InputMessage, SnapshotMessage,
                             ScoreboardMessage, SpawnMessage, DestroyMessage, PlayerDisconnectedMessage,
                             ScoreEventMessage, DeathEventMessage, GameOverEventMessage, PickUpEventMessage>;

// id u32 + type u8 + x f32 + y f32 + health u8
inline constexpr std::size_t ENTITY_STATE_BYTES = 14;

// The message type, the tick, the last input, the chunk index, the chunk count, the total entities and the entities
// of this chunk
inline constexpr std::size_t SNAPSHOT_HEADER_BYTES = 1 + 4 + 4 + 1 + 1 + 2 + 2;

// The entities that fit in one datagram, after the packet header and the header of the snapshot
inline constexpr std::size_t MAX_SNAPSHOT_ENTITIES =
    (younity::network::MAX_PACKET_BYTES - younity::network::PACKET_HEADER_BYTES - SNAPSHOT_HEADER_BYTES) /
    ENTITY_STATE_BYTES;

// The datagrams one snapshot is split into at most
inline constexpr std::size_t MAX_SNAPSHOT_CHUNKS = 64;

// The entities one snapshot can carry, over all its chunks
inline constexpr std::size_t MAX_SNAPSHOT_TOTAL_ENTITIES = MAX_SNAPSHOT_ENTITIES * MAX_SNAPSHOT_CHUNKS;

/**
 * @brief Compare two steps of the server, the count wrapping after 2^32 steps, about two years at 60 steps a second.
 *
 * @param first The step to test
 * @param second The step to compare it to
 *
 * @return true if first is after second.
 */
[[nodiscard]] constexpr bool isNewerTick(std::uint32_t first, std::uint32_t second) noexcept
{
    return static_cast<std::int32_t>(first - second) > 0;
}

/**
 * @brief Get the step of an event: a Spawn, a Destroy, a PlayerDisconnected, a ScoreEvent, a DeathEvent or a
 *        GameOverEvent, which a client plays when the world it draws reaches that step.
 *
 * @param message A message received from the server
 *
 * @return The step of the event, or nothing for a message that is not an event.
 */
[[nodiscard]] std::optional<std::uint32_t> getEventTick(const Message &message) noexcept;

/**
 * @brief Split a snapshot of a step into the chunks that fit in a datagram, numbering them.
 *        The chunks share the tick, the last input, the total number of entities and their count, and each carries
 *        its part.
 *
 * @param snapshot The snapshot of one step, with all its entities
 *
 * @return The chunks, one at least; empty if the snapshot has more than MAX_SNAPSHOT_TOTAL_ENTITIES entities.
 */
[[nodiscard]] std::vector<SnapshotMessage> splitSnapshot(SnapshotMessage snapshot);

/**
 * @brief Check that a snapshot chunk has the layout splitSnapshot() gives: its index below its count, the count of
 *        chunks its total of entities needs, and the entities of its place. The decoder checks a received chunk with
 *        it before reading its entities, the encoder before writing them.
 *
 * @param chunk The chunk, whose entities are not looked at
 * @param carried The number of entities the chunk carries
 *
 * @return true if its fields and its entities agree.
 */
[[nodiscard]] bool isValidChunk(const SnapshotMessage &chunk, std::size_t carried) noexcept;

/**
 * @brief Encode a message in a GameData packet, with Writer.
 *        A snapshot is encoded as the chunk it is, so split it with splitSnapshot() before.
 *
 * @param message The message to send
 * @param seq The sequence number of the packet
 *
 * @return The whole datagram, to give to UdpPump::sendTo; empty if the message does not fit in a packet.
 */
[[nodiscard]] std::vector<std::byte> encodeMessage(const Message &message, std::uint32_t seq);

/**
 * @brief Decode the message of a received packet, with Reader.
 *
 * @param packet The packet, decoded once with decodePacket for the session too
 *
 * @return The message, or the reason the packet carries none.
 */
[[nodiscard]] std::expected<Message, DecodeError> decodeMessage(const younity::network::Packet &packet);

} // namespace jtype
