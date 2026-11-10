/*
** EPITECH PROJECT, 2026
** j-type/common/src/Spawn
** File description:
** Spawn source file
*/

#include "jtype/common/Spawn.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <optional>
#include <vector>

#include "jtype/common/Constants.hpp"
#include "jtype/common/Gameplay.hpp"
#include "jtype/common/components/Boss.hpp"
#include "jtype/common/components/Enemy.hpp"
#include "jtype/common/components/Explosion.hpp"
#include "jtype/common/components/Health.hpp"
#include "jtype/common/components/Lifetime.hpp"
#include "jtype/common/components/Missile.hpp"
#include "jtype/common/components/Obstacle.hpp"
#include "jtype/common/components/Player.hpp"
#include "jtype/common/components/PowerUp.hpp"
#include "jtype/common/components/Score.hpp"
#include "younity/ecs/components/Transform.hpp"
#include "younity/ecs/components/Velocity.hpp"
#include "younity/physics/components/Hitbox.hpp"

namespace jtype {

using younity::ecs::Entity;
using younity::ecs::Registry;
using younity::ecs::Transform;
using younity::ecs::Velocity;
using younity::physics::Hitbox;

namespace {

/**
 * @brief Get the heights of the pieces of a wall or of a zapper, one every OBSTACLE_SIZE, between the ceiling and the
 *        floor.
 *
 * @param top The height of the first piece, from 0, against the ceiling, to 1, against the floor
 * @param bottom The height of the last piece
 *
 * @return The heights, from the top, at least one.
 */
std::vector<float> getObstacleHeights(float top, float bottom)
{
    constexpr float FIRST = TERRAIN_HEIGHT + (OBSTACLE_SIZE.y / 2.0F);
    constexpr float RANGE = WORLD_HEIGHT - (2.0F * TERRAIN_HEIGHT) - OBSTACLE_SIZE.y;
    const float from = FIRST + (top * RANGE);
    const float to = FIRST + (bottom * RANGE);
    // Rounded up, the last piece put back on the bottom height: no gap a ship could fly through
    const auto count = static_cast<std::size_t>(std::ceil((to - from) / OBSTACLE_SIZE.y)) + 1;
    std::vector<float> heights;

    for (std::size_t piece = 0; piece < count; ++piece) {
        heights.push_back(std::min(from + (static_cast<float>(piece) * OBSTACLE_SIZE.y), to));
    }
    return heights;
}

/**
 * @brief Create a piece of an obstacle just past the right edge, scrolling to the left with the stage.
 *
 * @param registry The registry of the game
 * @param y The height of the piece
 * @param kind What the piece is
 * @param hitbox Its box
 *
 * @return The piece.
 */
Entity spawnObstacle(Registry &registry, float y, ObstacleKind kind, Hitbox hitbox)
{
    const Entity piece = registry.createEntity();

    registry.addComponent(piece, Transform{
                                     {WORLD_WIDTH + (OBSTACLE_SIZE.x / 2.0F), y}
    });
    registry.addComponent(piece, Velocity{
                                     {-STAGE_SPEED, 0.0F}
    });
    registry.addComponent(piece, hitbox);
    registry.addComponent(piece, Obstacle{kind});
    return piece;
}

} // namespace

younity::Vector2f getStartPosition(std::uint8_t slot)
{
    return {PLAYER_START_X, WORLD_HEIGHT * static_cast<float>(slot + 1) / static_cast<float>(MAX_PLAYERS + 1)};
}

int getMaxHealth(EnemyType type)
{
    switch (type) {
    case EnemyType::Jelly:
        return 2;
    case EnemyType::SentryDrone:
        return 3;
    case EnemyType::Blobling:
        break;
    }
    return 1;
}

std::uint32_t getKillPoints(EnemyType type)
{
    return static_cast<std::uint32_t>(getMaxHealth(type)) * 100U;
}

bool canShoot(EnemyType type)
{
    return type != EnemyType::Blobling;
}

Entity spawnExplosion(Registry &registry, younity::Vector2f position)
{
    const Entity explosion = registry.createEntity();

    registry.addComponent(explosion, Transform{position});
    registry.addComponent(explosion, Lifetime{EXPLOSION_DURATION});
    registry.addComponent(explosion, Explosion{});
    return explosion;
}

Entity spawnPlayer(Registry &registry, std::uint8_t slot, std::uint8_t lives)
{
    const Entity ship = registry.createEntity();

    registry.addComponent(ship, Transform{getStartPosition(slot)});
    registry.addComponent(ship, Hitbox{PLAYER_HITBOX_SIZE, PLAYER_HITBOX_OFFSET});
    registry.addComponent(ship, Player{slot, 0, 0.0F, 0.0F, lives});
    registry.addComponent(ship, Score{});
    registry.addComponent(ship, Health{PLAYER_HEALTH, PLAYER_HEALTH});
    return ship;
}

Entity spawnMissile(Registry &registry, younity::Vector2f position, std::uint8_t shooter)
{
    const Entity missile = registry.createEntity();

    registry.addComponent(missile, Transform{position});
    registry.addComponent(missile, Velocity{
                                       {MISSILE_SPEED, 0.0F}
    });
    registry.addComponent(missile, Hitbox{MISSILE_HITBOX_SIZE, MISSILE_HITBOX_OFFSET});
    registry.addComponent(missile, Missile{shooter});
    return missile;
}

Entity spawnPellet(Registry &registry, younity::Vector2f position, younity::Vector2f direction, std::uint8_t shooter)
{
    const Entity pellet = registry.createEntity();

    registry.addComponent(pellet, Transform{position});
    registry.addComponent(pellet, Velocity{direction * MISSILE_SPEED});
    registry.addComponent(pellet, Hitbox{PELLET_HITBOX_SIZE, PELLET_HITBOX_OFFSET});
    registry.addComponent(pellet, Missile{shooter, 1, MissileKind::Pellet});
    return pellet;
}

Entity spawnPowerUp(Registry &registry, younity::Vector2f position, PowerUpType type)
{
    const Entity powerUp = registry.createEntity();

    registry.addComponent(powerUp, Transform{position});
    registry.addComponent(powerUp, Velocity{
                                       {-POWER_UP_SPEED, 0.0F}
    });
    registry.addComponent(powerUp, Hitbox{POWER_UP_HITBOX_SIZE, POWER_UP_HITBOX_OFFSET});
    registry.addComponent(powerUp, PowerUp{type});
    return powerUp;
}

void spawnTerrain(Registry &registry)
{
    const younity::Vector2f size{WORLD_WIDTH, TERRAIN_HEIGHT};

    for (const float center : {TERRAIN_HEIGHT / 2.0F, WORLD_HEIGHT - (TERRAIN_HEIGHT / 2.0F)}) {
        const Entity strip = registry.createEntity();

        registry.addComponent(strip, Transform{
                                         {WORLD_WIDTH / 2.0F, center}
        });
        registry.addComponent(strip, Hitbox{size, size / -2.0F});
        registry.addComponent(strip, Obstacle{ObstacleKind::Terrain});
    }
}

std::vector<Entity> spawnWall(Registry &registry, float top, float bottom)
{
    std::vector<Entity> blocks;

    for (const float y : getObstacleHeights(top, bottom)) {
        const Entity block = spawnObstacle(registry, y, ObstacleKind::Block, Hitbox{BLOCK_HITBOX_SIZE, BLOCK_HITBOX_OFFSET});

        registry.addComponent(block, Health{BLOCK_HEALTH, BLOCK_HEALTH});
        blocks.push_back(block);
    }
    return blocks;
}

std::vector<Entity> spawnZapper(Registry &registry, float top, float bottom)
{
    const std::vector<float> heights = getObstacleHeights(top, bottom);
    std::vector<Entity> pieces;

    for (std::size_t piece = 0; piece < heights.size(); ++piece) {
        const bool isNode = piece == 0 || piece + 1 == heights.size();

        pieces.push_back(isNode ? spawnObstacle(registry, heights[piece], ObstacleKind::ZapperNode,
                                                Hitbox{ZAPPER_NODE_HITBOX_SIZE, ZAPPER_NODE_HITBOX_OFFSET})
                                : spawnObstacle(registry, heights[piece], ObstacleKind::ZapperBeam,
                                                Hitbox{ZAPPER_BEAM_HITBOX_SIZE, ZAPPER_BEAM_HITBOX_OFFSET}));
    }
    return pieces;
}

Entity spawnEnemyMissile(Registry &registry, younity::Vector2f position, younity::Vector2f velocity, MissileKind kind)
{
    const Entity missile = registry.createEntity();

    registry.addComponent(missile, Transform{position});
    registry.addComponent(missile, Velocity{velocity});
    registry.addComponent(missile, Hitbox{ENEMY_MISSILE_HITBOX_SIZE, ENEMY_MISSILE_HITBOX_OFFSET});
    registry.addComponent(missile, Missile{std::nullopt, 1, kind});
    return missile;
}

MovementPattern getMovementPattern(EnemyType type)
{
    switch (type) {
    case EnemyType::Jelly:
        return MovementPattern::SineWave;
    case EnemyType::SentryDrone:
        return MovementPattern::Chase;
    case EnemyType::Blobling:
        break;
    }
    return MovementPattern::Straight;
}

Entity spawnBydo(Registry &registry, younity::Vector2f position, EnemyType type)
{
    return spawnBydo(registry, position, type, getMovementPattern(type));
}

Entity spawnBydo(Registry &registry, younity::Vector2f position, EnemyType type, MovementPattern pattern)
{
    const Entity bydo = registry.createEntity();

    registry.addComponent(bydo, Transform{position});
    registry.addComponent(bydo, Velocity{
                                    {-BYDO_SPEED, 0.0F}
    });
    registry.addComponent(bydo, Hitbox{BYDO_HITBOX_SIZE, BYDO_HITBOX_OFFSET});
    registry.addComponent(bydo, Enemy{type, pattern, ENEMY_FIRST_SHOT_DELAY, 0.0F});
    registry.addComponent(bydo, Health{getMaxHealth(type), getMaxHealth(type)});
    return bydo;
}

Entity spawnIncubator(Registry &registry, std::size_t players)
{
    const int health = BOSS_HEALTH + (BOSS_HEALTH_PER_PLAYER * static_cast<int>(std::max<std::size_t>(players, 1) - 1));
    const younity::Vector2f position{WORLD_WIDTH + (BOSS_SIZE.x / 2.0F), WORLD_HEIGHT / 2.0F};
    const Entity body = registry.createEntity();
    const Entity eye = registry.createEntity();

    registry.addComponent(body, Transform{position});
    registry.addComponent(body, Hitbox{BOSS_BODY_HITBOX_SIZE, BOSS_BODY_HITBOX_OFFSET});
    // The spits wait a little more than the orbs, so the first two attacks do not come together
    registry.addComponent(body, Boss{BossPart::Body, 0.0F, BOSS_ORB_DELAY, BOSS_SPIT_DELAY / 2.0F});
    registry.addComponent(eye, Transform{position + BOSS_EYE_OFFSET});
    registry.addComponent(eye, Hitbox{BOSS_EYE_HITBOX_SIZE, BOSS_EYE_HITBOX_OFFSET});
    registry.addComponent(eye, Boss{BossPart::Eye});
    registry.addComponent(eye, Health{health, health});
    return body;
}

Entity spawnBossWreck(Registry &registry, younity::Vector2f position)
{
    const Entity wreck = registry.createEntity();

    registry.addComponent(wreck, Transform{position});
    registry.addComponent(wreck, Lifetime{BOSS_WRECK_DURATION});
    registry.addComponent(wreck, BossWreck{});
    return wreck;
}

} // namespace jtype
