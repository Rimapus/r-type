/*
** EPITECH PROJECT, 2026
** j-type/client/src/audio/SoundCues
** File description:
** SoundCues header file
*/

#pragma once

#include <optional>

#include "jtype/common/EntityType.hpp"
#include "jtype/common/Messages.hpp"

namespace jtype {

/**
 * @brief A sound of the game.
 */
enum class Sound {
    PlayerShot,
    EnemyShot,
    Explosion,
    PlayerHit,
    PlayerOut,
    GameOver
};

/**
 * @brief Find the sound of an event of the server: a shot or an explosion that appears, a ship destroyed, the end of
 *        a lost game. The event is played when the world drawn reaches its step, so the sound comes with what is drawn.
 *
 * @param event The event
 * @param world The world drawn before the event, which still has the ship a DeathEvent names
 * @param scoreboard The last scoreboard, which tells if the player of a destroyed ship is out of lives, and if the game
 *        ended with a victory
 *
 * @return The sound, or nothing for an event that plays none.
 */
[[nodiscard]] std::optional<Sound> getEventSound(const Message &event, const SnapshotMessage &world,
                                                 const ScoreboardMessage &scoreboard);

} // namespace jtype
