/*
** EPITECH PROJECT, 2026
** j-type/common/include/jtype/common/Spawn
** File description:
** Spawn header file
*/

#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "jtype/common/Gameplay.hpp"
#include "jtype/common/PowerUpType.hpp"
#include "jtype/common/components/Enemy.hpp"
#include "jtype/common/components/Missile.hpp"
#include "younity/core/Vector2.hpp"
#include "younity/ecs/Entity.hpp"
#include "younity/ecs/Registry.hpp"

namespace jtype {

/**
 * @brief Get where the ship of a player starts.
 *
 * @param slot The slot of the player
 *
 * @return On the left of the screen, at a height given by the slot.
 */
[[nodiscard]] younity::Vector2f getStartPosition(std::uint8_t slot);

/**
 * @brief Get the health points of a type of Bydo: 1 for a Blobling, 2 for a Jelly, 3 for a SentryDrone.
 *
 * @param type The type of the Bydo
 *
 * @return Its health when it appears.
 */
[[nodiscard]] int getMaxHealth(EnemyType type);

/**
 * @brief Get the points a player earns by destroying a type of Bydo: 100, 200 or 300.
 *
 * @param type The type of the Bydo
 *
 * @return The points.
 */
[[nodiscard]] std::uint32_t getKillPoints(EnemyType type);

/**
 * @brief Check if a type of Bydo shoots: a Jelly and a SentryDrone do, a Blobling only flies, so the first waves
 *        teach the movement before the shots.
 *
 * @param type The type of the Bydo
 *
 * @return true if it fires missiles.
 */
[[nodiscard]] bool canShoot(EnemyType type);

/**
 * @brief Create an explosion, shown for EXPLOSION_DURATION.
 *
 * @param registry The registry of the game
 * @param position Where it is
 *
 * @return The explosion.
 */
younity::ecs::Entity spawnExplosion(younity::ecs::Registry &registry, younity::Vector2f position);

/**
 * @brief Create the ship of a player at its start position, with PLAYER_HEALTH and no score.
 *
 * @param registry The registry of the game
 * @param slot The slot of the player
 * @param lives The lives it starts with, those of the difficulty
 *
 * @return The ship.
 */
younity::ecs::Entity spawnPlayer(younity::ecs::Registry &registry, std::uint8_t slot,
                                 std::uint8_t lives = PLAYER_LIVES);

/**
 * @brief Create a missile of a player going right in a straight line.
 *
 * @param registry The registry of the game
 * @param position Where the missile starts
 * @param shooter The slot of the player who shot it
 *
 * @return The missile.
 */
younity::ecs::Entity spawnMissile(younity::ecs::Registry &registry, younity::Vector2f position, std::uint8_t shooter);

/**
 * @brief Create a pellet of the spread shot of a player.
 *
 * @param registry The registry of the game
 * @param position Where the pellet starts
 * @param direction Where it goes, a unit vector: it flies at MISSILE_SPEED
 * @param shooter The slot of the player who shot it
 *
 * @return The pellet.
 */
younity::ecs::Entity spawnPellet(younity::ecs::Registry &registry, younity::Vector2f position, younity::Vector2f direction,
                                 std::uint8_t shooter);

/**
 * @brief Create a power-up drifting to the left at POWER_UP_SPEED.
 *
 * @param registry The registry of the game
 * @param position Where it appears, e.g. where a Bydo was destroyed
 * @param type What it gives the ship that takes it
 *
 * @return The power-up.
 */
younity::ecs::Entity spawnPowerUp(younity::ecs::Registry &registry, younity::Vector2f position, PowerUpType type);

/**
 * @brief Create the ceiling and the floor, TERRAIN_HEIGHT high along the whole width of the screen. They do not move.
 *
 * @param registry The registry of the game
 */
void spawnTerrain(younity::ecs::Registry &registry);

/**
 * @brief Create a wall of blocks just past the right edge, scrolling to the left at STAGE_SPEED: one block every
 *        OBSTACLE_SIZE from a height to another, each with BLOCK_HEALTH.
 *
 * @param registry The registry of the game
 * @param top The height of the first block, from 0, against the ceiling, to 1, against the floor
 * @param bottom The height of the last block, not above the first one
 *
 * @return The blocks, from the top.
 */
std::vector<younity::ecs::Entity> spawnWall(younity::ecs::Registry &registry, float top, float bottom);

/**
 * @brief Create a zapper just past the right edge, scrolling to the left at STAGE_SPEED: a node at each height and a
 *        beam between them, one piece every OBSTACLE_SIZE. Nothing breaks it.
 *
 * @param registry The registry of the game
 * @param top The height of the top node, from 0, against the ceiling, to 1, against the floor
 * @param bottom The height of the bottom node, not above the top one
 *
 * @return The pieces, from the top.
 */
std::vector<younity::ecs::Entity> spawnZapper(younity::ecs::Registry &registry, float top, float bottom);

/**
 * @brief Create a missile of a Bydo or of the boss.
 *
 * @param registry The registry of the game
 * @param position Where the missile starts
 * @param velocity Its speed and direction, in pixels per second
 * @param kind What fired it, so the players see it: an orb of a Bydo by default
 *
 * @return The missile.
 */
younity::ecs::Entity spawnEnemyMissile(younity::ecs::Registry &registry, younity::Vector2f position, younity::Vector2f velocity,
                                       MissileKind kind = MissileKind::Orb);

/**
 * @brief Create the Incubator just past the right edge, in the middle of the screen: its body, and its eye with
 *        BOSS_HEALTH, plus BOSS_HEALTH_PER_PLAYER for each player after the first.
 *
 * @param registry The registry of the game
 * @param players The players in the game, at least one counts
 *
 * @return The body.
 */
younity::ecs::Entity spawnIncubator(younity::ecs::Registry &registry, std::size_t players);

/**
 * @brief Create the wreck of the boss, shown for BOSS_WRECK_DURATION.
 *
 * @param registry The registry of the game
 * @param position Where the body was
 *
 * @return The wreck.
 */
younity::ecs::Entity spawnBossWreck(younity::ecs::Registry &registry, younity::Vector2f position);

/**
 * @brief Get how a type of Bydo moves: a Blobling goes straight, a Jelly waves, a SentryDrone chases the players.
 *
 * @param type The type of the Bydo
 *
 * @return Its movement pattern.
 */
[[nodiscard]] MovementPattern getMovementPattern(EnemyType type);

/**
 * @brief Create a Bydo moving with the pattern of its type, shooting for the first time ENEMY_FIRST_SHOT_DELAY after
 *        it enters the screen if its type shoots.
 *
 * @param registry The registry of the game
 * @param position Where the Bydo starts
 * @param type The type of the Bydo
 *
 * @return The Bydo.
 */
younity::ecs::Entity spawnBydo(younity::ecs::Registry &registry, younity::Vector2f position, EnemyType type);

/**
 * @brief Create a Bydo moving with a given pattern, e.g. the one of a wave, shooting for the first time
 *        ENEMY_FIRST_SHOT_DELAY after it enters the screen if its type shoots.
 *
 * @param registry The registry of the game
 * @param position Where the Bydo starts
 * @param type The type of the Bydo
 * @param pattern How it moves
 *
 * @return The Bydo.
 */
younity::ecs::Entity spawnBydo(younity::ecs::Registry &registry, younity::Vector2f position, EnemyType type,
                               MovementPattern pattern);

} // namespace jtype
