/*
** EPITECH PROJECT, 2026
** j-type/client/src/audio/SoundCues
** File description:
** SoundCues source file
*/

#include "audio/SoundCues.hpp"

#include <algorithm>
#include <cstdint>
#include <variant>

#include "jtype/common/EntityType.hpp"

namespace jtype {

namespace {

/**
 * @brief Find the sound of an entity that appeared.
 *
 * @param type The type of the entity
 *
 * @return Its sound, or nothing for an entity that appears silently.
 */
std::optional<Sound> getAppearSound(EntityType type)
{
    switch (type) {
    case EntityType::Missile1:
    case EntityType::Missile2:
    case EntityType::Missile3:
    case EntityType::Missile4:
    case EntityType::SpreadPellet1:
    case EntityType::SpreadPellet2:
    case EntityType::SpreadPellet3:
    case EntityType::SpreadPellet4:
        return Sound::PlayerShot;
    case EntityType::EnemyMissile:
    case EntityType::IncubatorOrb:
    case EntityType::IncubatorSpit:
        return Sound::EnemyShot;
    case EntityType::Explosion:
    case EntityType::IncubatorWreck:
        return Sound::Explosion;
    case EntityType::Player1:
    case EntityType::Player2:
    case EntityType::Player3:
    case EntityType::Player4:
    case EntityType::Blobling:
    case EntityType::Jelly:
    case EntityType::SentryDrone:
    case EntityType::IncubatorBody:
    case EntityType::IncubatorEye:
    case EntityType::IncubatorEyeAngry:
    case EntityType::ExtraLifePowerUp:
    case EntityType::ShieldPowerUp:
    case EntityType::SpreadShotPowerUp:
    case EntityType::Block:
    case EntityType::CrackedBlock:
    case EntityType::ZapperNode:
    case EntityType::ZapperBeam:
        break;
    }
    return std::nullopt;
}

} // namespace

std::optional<Sound> getEventSound(const Message &event, const SnapshotMessage &world, const ScoreboardMessage &scoreboard)
{
    if (const auto *spawn = std::get_if<SpawnMessage>(&event)) {
        return getAppearSound(spawn->entity.type);
    }
    // The sound of the game over is the one of a lost game: a victory ends on the explosion of the boss
    if (std::holds_alternative<GameOverEventMessage>(event)) {
        return scoreboard.victory ? std::nullopt : std::optional(Sound::GameOver);
    }
    const auto *death = std::get_if<DeathEventMessage>(&event);

    if (death == nullptr) {
        return std::nullopt;
    }
    const auto ship = std::ranges::find(world.entities, death->id, &EntityState::id);
    const std::optional<std::uint8_t> slot = ship == world.entities.end() ? std::nullopt : getPlayerSlot(ship->type);

    // A Bydo destroyed plays the explosion that appears with it
    if (!slot) {
        return std::nullopt;
    }
    return scoreboard.players[*slot].state == SlotState::Out ? Sound::PlayerOut : Sound::PlayerHit;
}

} // namespace jtype
