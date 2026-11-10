/*
** EPITECH PROJECT, 2026
** j-type/common/src/Messages
** File description:
** Messages source file
*/

#include "jtype/common/Messages.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <iterator>
#include <optional>
#include <span>
#include <type_traits>
#include <utility>

#include "jtype/common/Input.hpp"
#include "younity/network/CodecError.hpp"
#include "younity/network/Reader.hpp"
#include "younity/network/Writer.hpp"

namespace jtype {

namespace {

// The only bits a client may send, see Input.hpp
constexpr std::uint8_t INPUT_BITS = input::UP | input::DOWN | input::LEFT | input::RIGHT | input::SHOOT;

/**
 * @brief Get the number of chunks a snapshot is split into.
 *
 * @param total The number of entities of the whole snapshot
 *
 * @return The number of chunks, 1 for an empty snapshot.
 */
std::size_t chunkCountOf(std::size_t total) noexcept
{
    return std::max<std::size_t>(1, (total + MAX_SNAPSHOT_ENTITIES - 1) / MAX_SNAPSHOT_ENTITIES);
}

/**
 * @brief Get the number of entities a chunk carries.
 *
 * @param total The number of entities of the whole snapshot
 * @param index The place of the chunk
 *
 * @return The number of entities of the chunk, 0 past the end.
 */
std::size_t chunkSizeOf(std::size_t total, std::size_t index) noexcept
{
    const std::size_t offset = index * MAX_SNAPSHOT_ENTITIES;

    return offset < total ? std::min(MAX_SNAPSHOT_ENTITIES, total - offset) : 0;
}

/**
 * @brief Translate the failure of a field of the engine into the reason a game message is dropped.
 *
 * @param error The engine error
 *
 * @return Truncated when the field is missing, InvalidValue when it holds something impossible.
 */
DecodeError toDecodeError(younity::network::CodecError error) noexcept
{
    return error == younity::network::CodecError::Truncated ? DecodeError::Truncated : DecodeError::InvalidValue;
}

/**
 * @brief Read the fields of a message, remembering the first one that was impossible to read.
 *        The reads go on so a message is written in one block: the caller checks error() once at the end.
 */
class Payload
{
public:
    explicit Payload(std::span<const std::byte> bytes) noexcept
        : _reader(bytes)
    {
    }

    [[nodiscard]] std::uint8_t u8(void) noexcept
    {
        return this->take(this->_reader.getU8());
    }

    [[nodiscard]] std::uint16_t u16(void) noexcept
    {
        return this->take(this->_reader.getU16Le());
    }

    [[nodiscard]] std::uint32_t u32(void) noexcept
    {
        return this->take(this->_reader.getU32Le());
    }

    [[nodiscard]] float f32(void) noexcept
    {
        return this->take(this->_reader.getFloatLe());
    }

    /**
     * @brief Refuse the message with a reason of its own, e.g. a value out of its range.
     *
     * @param error The reason to keep if no field failed before
     */
    void reject(DecodeError error) noexcept
    {
        if (!this->_error) {
            this->_error = error;
        }
    }

    /**
     * @brief Get the reason the payload is invalid.
     *
     * @return The reason, or nothing if every read succeeded.
     */
    [[nodiscard]] std::optional<DecodeError> error(void) const noexcept
    {
        return this->_error;
    }

    /**
     * @brief Get the number of bytes left after the message.
     *
     * @return The number of unread bytes.
     */
    [[nodiscard]] std::size_t remaining(void) const noexcept
    {
        return this->_reader.getRemaining();
    }

private:
    template <typename T>
    [[nodiscard]] T take(std::expected<T, younity::network::CodecError> value) noexcept
    {
        if (!value) {
            this->reject(toDecodeError(value.error()));
            return T{};
        }
        return *value;
    }

    younity::network::Reader _reader;
    std::optional<DecodeError> _error;
};

/**
 * @brief Check that a value is one of the types of the entities.
 *
 * @param type The value read from the message
 *
 * @return true if it names an entity type.
 */
bool isValidEntityType(EntityType type) noexcept
{
    switch (type) {
    case EntityType::Player1:
    case EntityType::Player2:
    case EntityType::Player3:
    case EntityType::Player4:
    case EntityType::Blobling:
    case EntityType::Missile1:
    case EntityType::EnemyMissile:
    case EntityType::Jelly:
    case EntityType::SentryDrone:
    case EntityType::Explosion:
    case EntityType::Missile2:
    case EntityType::Missile3:
    case EntityType::Missile4:
    case EntityType::IncubatorBody:
    case EntityType::IncubatorEye:
    case EntityType::IncubatorEyeAngry:
    case EntityType::IncubatorOrb:
    case EntityType::IncubatorSpit:
    case EntityType::IncubatorWreck:
    case EntityType::SpreadPellet1:
    case EntityType::SpreadPellet2:
    case EntityType::SpreadPellet3:
    case EntityType::SpreadPellet4:
    case EntityType::ExtraLifePowerUp:
    case EntityType::ShieldPowerUp:
    case EntityType::SpreadShotPowerUp:
    case EntityType::Block:
    case EntityType::CrackedBlock:
    case EntityType::ZapperNode:
    case EntityType::ZapperBeam:
        return true;
    }
    return false;
}

/**
 * @brief Read an entity of a snapshot or of a Spawn event.
 *
 * @param in The reader, placed on the entity
 *
 * @return The entity; check the error of the reader for its type and its position.
 */
EntityState readEntityState(Payload &in) noexcept
{
    EntityState entity;

    entity.id = in.u32();
    entity.type = static_cast<EntityType>(in.u8());
    entity.x = in.f32();
    entity.y = in.f32();
    entity.health = in.u8();
    if (!isValidEntityType(entity.type) || !std::isfinite(entity.x) || !std::isfinite(entity.y)) {
        in.reject(DecodeError::InvalidValue);
    }
    return entity;
}

/**
 * @brief Write an entity of a snapshot or of a Spawn event.
 *
 * @param entity The entity to write
 * @param out The writer receiving it
 *
 * @return false if it does not fit in a packet.
 */
bool writeEntityState(const EntityState &entity, younity::network::Writer &out)
{
    return out.putU32Le(entity.id) && out.putU8(static_cast<std::uint8_t>(entity.type)) && out.putFloatLe(entity.x) &&
           out.putFloatLe(entity.y) && out.putU8(entity.health);
}

/**
 * @brief Refuse a message that has more bytes than its fields.
 *
 * @param in The reader, placed after the fields
 * @param message The message to return
 *
 * @return The message, or TrailingBytes.
 */
std::expected<Message, DecodeError> finish(Payload &in, Message message)
{
    if (const std::optional<DecodeError> error = in.error()) {
        return std::unexpected(*error);
    }
    if (in.remaining() != 0) {
        return std::unexpected(DecodeError::TrailingBytes);
    }
    return message;
}

/**
 * @brief Read a snapshot chunk, after its type.
 *
 * @param in The reader, placed after the message type
 *
 * @return The chunk, or the reason it is invalid.
 */
std::expected<Message, DecodeError> readSnapshot(Payload &in)
{
    SnapshotMessage snapshot;

    snapshot.tick = in.u32();
    snapshot.lastInput = in.u32();
    snapshot.chunkIndex = in.u8();
    snapshot.chunkCount = in.u8();
    snapshot.entityCount = in.u16();
    const std::uint16_t count = in.u16();
    if (const std::optional<DecodeError> error = in.error()) {
        return std::unexpected(*error);
    }
    // Checked before the entities are read, so a forged count never reserves memory
    if (!isValidChunk(snapshot, count)) {
        return std::unexpected(DecodeError::InvalidChunk);
    }
    snapshot.entities.reserve(count);
    for (std::uint16_t index = 0; index < count; ++index) {
        snapshot.entities.push_back(readEntityState(in));
    }
    return finish(in, std::move(snapshot));
}

/**
 * @brief Read a scoreboard, after its type.
 *
 * @param in The reader, placed after the message type
 *
 * @return The scoreboard, or the reason it is invalid.
 */
std::expected<Message, DecodeError> readScoreboard(Payload &in)
{
    ScoreboardMessage scoreboard;

    scoreboard.tick = in.u32();

    // 0 while the game runs, 1 once every player is out of lives, 2 once the boss is beaten
    const std::uint8_t end = in.u8();

    // A reader keeps the first error: a value read after a truncation is 0, which is valid
    if (end > 2) {
        in.reject(DecodeError::InvalidValue);
    }
    scoreboard.gameOver = end != 0;
    scoreboard.victory = end == 2;
    for (PlayerStatus &player : scoreboard.players) {
        const std::uint8_t state = in.u8();

        player.score = in.u32();
        player.lives = in.u8();

        const std::uint8_t weapon = in.u8();

        if (state > static_cast<std::uint8_t>(SlotState::Left) || weapon > static_cast<std::uint8_t>(Weapon::Spread)) {
            in.reject(DecodeError::InvalidValue);
        } else {
            player.state = static_cast<SlotState>(state);
            player.weapon = static_cast<Weapon>(weapon);
        }
    }
    return finish(in, std::move(scoreboard));
}

/**
 * @brief Read an input, after its type.
 *
 * @param in The reader, placed after the message type
 *
 * @return The input, or the reason it is invalid.
 */
std::expected<Message, DecodeError> readInput(Payload &in)
{
    InputMessage input;

    input.inputs = in.u8();
    input.tick = in.u32();
    if (!in.error() && (input.inputs & ~INPUT_BITS) != 0) {
        return std::unexpected(DecodeError::InvalidValue);
    }
    return finish(in, input);
}

/**
 * @brief Read a slot of the players, after the fields before it.
 *
 * @param in The reader, placed on the slot
 *
 * @return The slot, or MAX_PLAYERS when it is not one.
 */
std::uint8_t readSlot(Payload &in) noexcept
{
    const std::uint8_t slot = in.u8();

    if (!in.error() && slot >= MAX_PLAYERS) {
        in.reject(DecodeError::InvalidValue);
    }
    return slot;
}

/**
 * @brief Write the payload of a message: its type, then its fields.
 *
 * @param message The message to write
 * @param out The writer receiving the payload
 *
 * @return false if the payload does not fit in a packet.
 */
bool writePayload(const Message &message, younity::network::Writer &out)
{
    return std::visit(
        [&out](const auto &content) {
            using Content = std::decay_t<decltype(content)>;

            if constexpr (std::is_same_v<Content, JoinMessage>) {
                return out.putU8(static_cast<std::uint8_t>(MessageType::Join));
            } else if constexpr (std::is_same_v<Content, LeaveMessage>) {
                return out.putU8(static_cast<std::uint8_t>(MessageType::Leave));
            } else if constexpr (std::is_same_v<Content, WelcomeMessage>) {
                return out.putU8(static_cast<std::uint8_t>(MessageType::Welcome)) && out.putU8(content.slot);
            } else if constexpr (std::is_same_v<Content, FullMessage>) {
                return out.putU8(static_cast<std::uint8_t>(MessageType::Full));
            } else if constexpr (std::is_same_v<Content, InputMessage>) {
                return out.putU8(static_cast<std::uint8_t>(MessageType::Input)) && out.putU8(content.inputs) &&
                       out.putU32Le(content.tick);
            } else if constexpr (std::is_same_v<Content, ScoreboardMessage>) {
                const std::uint8_t end = content.gameOver ? (content.victory ? 2 : 1) : 0;
                bool written = out.putU8(static_cast<std::uint8_t>(MessageType::Scoreboard)) &&
                               out.putU32Le(content.tick) && out.putU8(end);

                for (const PlayerStatus &player : content.players) {
                    written = written && out.putU8(static_cast<std::uint8_t>(player.state)) &&
                              out.putU32Le(player.score) && out.putU8(player.lives) &&
                              out.putU8(static_cast<std::uint8_t>(player.weapon));
                }
                return written;
            } else if constexpr (std::is_same_v<Content, SnapshotMessage>) {
                if (!isValidChunk(content, content.entities.size())) {
                    return false;
                }
                bool written = out.putU8(static_cast<std::uint8_t>(MessageType::Snapshot)) &&
                               out.putU32Le(content.tick) && out.putU32Le(content.lastInput) &&
                               out.putU8(content.chunkIndex) &&
                               out.putU8(content.chunkCount) && out.putU16Le(content.entityCount) &&
                               out.putU16Le(static_cast<std::uint16_t>(content.entities.size()));

                for (const EntityState &entity : content.entities) {
                    written = written && writeEntityState(entity, out);
                }
                return written;
            } else if constexpr (std::is_same_v<Content, SpawnMessage>) {
                return out.putU8(static_cast<std::uint8_t>(MessageType::Spawn)) && out.putU32Le(content.tick) &&
                       writeEntityState(content.entity, out);
            } else if constexpr (std::is_same_v<Content, DestroyMessage>) {
                return out.putU8(static_cast<std::uint8_t>(MessageType::Destroy)) && out.putU32Le(content.tick) &&
                       out.putU32Le(content.id);
            } else if constexpr (std::is_same_v<Content, PlayerDisconnectedMessage>) {
                return out.putU8(static_cast<std::uint8_t>(MessageType::PlayerDisconnected)) &&
                       out.putU32Le(content.tick) && out.putU8(content.slot);
            } else if constexpr (std::is_same_v<Content, ScoreEventMessage>) {
                return out.putU8(static_cast<std::uint8_t>(MessageType::ScoreEvent)) && out.putU32Le(content.tick) &&
                       out.putU8(content.slot) && out.putU32Le(content.points);
            } else if constexpr (std::is_same_v<Content, DeathEventMessage>) {
                return out.putU8(static_cast<std::uint8_t>(MessageType::DeathEvent)) && out.putU32Le(content.tick) &&
                       out.putU32Le(content.id) && out.putU8(content.killer);
            } else if constexpr (std::is_same_v<Content, GameOverEventMessage>) {
                return out.putU8(static_cast<std::uint8_t>(MessageType::GameOverEvent)) && out.putU32Le(content.tick);
            } else if constexpr (std::is_same_v<Content, PickUpEventMessage>) {
                return out.putU8(static_cast<std::uint8_t>(MessageType::PickUpEvent)) && out.putU32Le(content.tick) &&
                       out.putU8(content.slot) && out.putU8(static_cast<std::uint8_t>(content.powerUp));
            } else {
                static_assert(!sizeof(Content), "a message of the protocol has no encoding");
            }
        },
        message);
}

} // namespace

std::vector<SnapshotMessage> splitSnapshot(SnapshotMessage snapshot)
{
    const std::size_t total = snapshot.entities.size();

    if (total > MAX_SNAPSHOT_TOTAL_ENTITIES) {
        return {};
    }
    const std::size_t count = chunkCountOf(total);
    std::vector<SnapshotMessage> chunks;

    chunks.reserve(count);
    for (std::size_t index = 0; index < count; ++index) {
        const std::size_t offset = index * MAX_SNAPSHOT_ENTITIES;
        const std::size_t end = std::min(offset + MAX_SNAPSHOT_ENTITIES, total);
        const auto first = snapshot.entities.begin() + static_cast<std::ptrdiff_t>(offset);
        const auto last = snapshot.entities.begin() + static_cast<std::ptrdiff_t>(end);
        SnapshotMessage chunk;

        chunk.tick = snapshot.tick;
        chunk.lastInput = snapshot.lastInput;
        chunk.chunkIndex = static_cast<std::uint8_t>(index);
        chunk.chunkCount = static_cast<std::uint8_t>(count);
        chunk.entityCount = static_cast<std::uint16_t>(total);
        chunk.entities.assign(std::make_move_iterator(first), std::make_move_iterator(last));
        chunks.push_back(std::move(chunk));
    }
    return chunks;
}

std::optional<std::uint32_t> getEventTick(const Message &message) noexcept
{
    return std::visit(
        [](const auto &content) -> std::optional<std::uint32_t> {
            using Content = std::decay_t<decltype(content)>;

            if constexpr (std::is_same_v<Content, SpawnMessage> || std::is_same_v<Content, DestroyMessage> ||
                          std::is_same_v<Content, PlayerDisconnectedMessage> ||
                          std::is_same_v<Content, ScoreEventMessage> || std::is_same_v<Content, DeathEventMessage> ||
                          std::is_same_v<Content, GameOverEventMessage> || std::is_same_v<Content, PickUpEventMessage>) {
                return content.tick;
            } else {
                return std::nullopt;
            }
        },
        message);
}

bool isValidChunk(const SnapshotMessage &chunk, std::size_t carried) noexcept
{
    const std::size_t total = chunk.entityCount;

    return chunk.chunkCount != 0 && chunk.chunkCount <= MAX_SNAPSHOT_CHUNKS && chunk.chunkIndex < chunk.chunkCount &&
           total <= MAX_SNAPSHOT_TOTAL_ENTITIES && chunk.chunkCount == chunkCountOf(total) &&
           carried == chunkSizeOf(total, chunk.chunkIndex);
}

std::vector<std::byte> encodeMessage(const Message &message, std::uint32_t seq)
{
    younity::network::Writer payload;
    younity::network::Writer datagram;
    younity::network::Packet packet;

    if (!writePayload(message, payload)) {
        return {};
    }
    packet.type = younity::network::PacketType::GameData;
    packet.seq = seq;
    packet.payload.assign(payload.getData().begin(), payload.getData().end());
    if (!younity::network::encodePacket(packet, datagram)) {
        return {};
    }
    return {datagram.getData().begin(), datagram.getData().end()};
}

std::expected<Message, DecodeError> decodeMessage(const younity::network::Packet &packet)
{
    if (packet.type != younity::network::PacketType::GameData) {
        return std::unexpected(DecodeError::NotGameData);
    }
    if (packet.payload.empty()) {
        return std::unexpected(DecodeError::Empty);
    }
    Payload in(packet.payload);

    const std::uint8_t type = in.u8();

    switch (type) {
    case static_cast<std::uint8_t>(MessageType::Join):
        return finish(in, JoinMessage{});
    case static_cast<std::uint8_t>(MessageType::Leave):
        return finish(in, LeaveMessage{});
    case static_cast<std::uint8_t>(MessageType::Welcome): {
        WelcomeMessage welcome;

        welcome.slot = readSlot(in);
        return finish(in, welcome);
    }
    case static_cast<std::uint8_t>(MessageType::Full):
        return finish(in, FullMessage{});
    case static_cast<std::uint8_t>(MessageType::Input):
        return readInput(in);
    case static_cast<std::uint8_t>(MessageType::Snapshot):
        return readSnapshot(in);
    case static_cast<std::uint8_t>(MessageType::Scoreboard):
        return readScoreboard(in);
    case static_cast<std::uint8_t>(MessageType::Spawn): {
        SpawnMessage spawn;

        spawn.tick = in.u32();
        spawn.entity = readEntityState(in);
        return finish(in, spawn);
    }
    case static_cast<std::uint8_t>(MessageType::Destroy): {
        DestroyMessage destroy;

        destroy.tick = in.u32();
        destroy.id = in.u32();
        return finish(in, destroy);
    }
    case static_cast<std::uint8_t>(MessageType::PlayerDisconnected): {
        PlayerDisconnectedMessage disconnected;

        disconnected.tick = in.u32();
        disconnected.slot = readSlot(in);
        return finish(in, disconnected);
    }
    case static_cast<std::uint8_t>(MessageType::ScoreEvent): {
        ScoreEventMessage score;

        score.tick = in.u32();
        score.slot = readSlot(in);
        score.points = in.u32();
        return finish(in, score);
    }
    case static_cast<std::uint8_t>(MessageType::DeathEvent): {
        DeathEventMessage death;

        death.tick = in.u32();
        death.id = in.u32();
        death.killer = in.u8();
        if (!in.error() && death.killer != NO_PLAYER && death.killer >= MAX_PLAYERS) {
            in.reject(DecodeError::InvalidValue);
        }
        return finish(in, death);
    }
    case static_cast<std::uint8_t>(MessageType::GameOverEvent): {
        GameOverEventMessage gameOver;

        gameOver.tick = in.u32();
        return finish(in, gameOver);
    }
    case static_cast<std::uint8_t>(MessageType::PickUpEvent): {
        PickUpEventMessage pickUp;

        pickUp.tick = in.u32();
        pickUp.slot = readSlot(in);

        const std::uint8_t powerUp = in.u8();

        if (powerUp > static_cast<std::uint8_t>(PowerUpType::SpreadShot)) {
            in.reject(DecodeError::InvalidValue);
        } else {
            pickUp.powerUp = static_cast<PowerUpType>(powerUp);
        }
        return finish(in, pickUp);
    }
    }
    return std::unexpected(DecodeError::UnknownType);
}

std::string_view toString(DecodeError error) noexcept
{
    switch (error) {
    case DecodeError::NotGameData:
        return "not a game message";
    case DecodeError::Empty:
        return "empty";
    case DecodeError::UnknownType:
        return "unknown type";
    case DecodeError::Truncated:
        return "truncated";
    case DecodeError::InvalidValue:
        return "invalid value";
    case DecodeError::TrailingBytes:
        return "trailing bytes";
    case DecodeError::InvalidChunk:
        return "invalid chunk";
    }
    return "unknown";
}

} // namespace jtype
