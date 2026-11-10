/*
** EPITECH PROJECT, 2026
** j-type/tests/common/Spawn
** File description:
** Spawn unit tests
*/

#include <criterion/criterion.h>

#include <vector>

#include "jtype/common/Constants.hpp"
#include "jtype/common/Gameplay.hpp"
#include "jtype/common/Spawn.hpp"
#include "jtype/common/components/Health.hpp"
#include "jtype/common/components/Obstacle.hpp"
#include "jtype/common/components/Player.hpp"
#include "younity/ecs/Registry.hpp"
#include "younity/ecs/components/Transform.hpp"
#include "younity/ecs/components/Velocity.hpp"
#include "younity/physics/components/Hitbox.hpp"

using jtype::Obstacle;
using jtype::ObstacleKind;
using younity::Vector2f;
using younity::ecs::Entity;
using younity::ecs::Registry;
using younity::ecs::Transform;
using younity::physics::Hitbox;

namespace {

Vector2f positionOf(const Registry &registry, Entity entity)
{
    return registry.getComponent<Transform>(entity).position;
}

} // namespace

Test(common_spawn, terrain_covers_the_top_and_the_bottom_of_the_screen)
{
    Registry registry;

    jtype::spawnTerrain(registry);

    const std::vector<Entity> strips = registry.getEntitiesWith<Obstacle>();

    cr_assert_eq(strips.size(), 2U);
    for (const Entity strip : strips) {
        const Hitbox &hitbox = registry.getComponent<Hitbox>(strip);
        const float top = positionOf(registry, strip).y + hitbox.offset.y;

        cr_assert(registry.getComponent<Obstacle>(strip).kind == ObstacleKind::Terrain);
        cr_assert_float_eq(hitbox.size.x, jtype::WORLD_WIDTH, 1e-3F);
        cr_assert(top == 0.0F || top == jtype::WORLD_HEIGHT - jtype::TERRAIN_HEIGHT);
    }
}

Test(common_spawn, wall_stacks_its_blocks_from_the_ceiling_to_the_floor)
{
    Registry registry;
    const std::vector<Entity> blocks = jtype::spawnWall(registry, 0.0F, 1.0F);

    cr_assert_float_eq(positionOf(registry, blocks.front()).y, jtype::TERRAIN_HEIGHT + (jtype::OBSTACLE_SIZE.y / 2.0F), 1e-3F);
    cr_assert_float_eq(positionOf(registry, blocks.back()).y, jtype::WORLD_HEIGHT - jtype::TERRAIN_HEIGHT - (jtype::OBSTACLE_SIZE.y / 2.0F),
                       1e-3F);
    for (std::size_t block = 0; block < blocks.size(); ++block) {
        cr_assert_gt(positionOf(registry, blocks[block]).x, jtype::WORLD_WIDTH);
        cr_assert(registry.getComponent<younity::ecs::Velocity>(blocks[block]).value == (Vector2f{-jtype::STAGE_SPEED, 0.0F}));
        cr_assert_eq(registry.getComponent<jtype::Health>(blocks[block]).current, jtype::BLOCK_HEALTH);
        // No gap a ship could fly through
        if (block > 0) {
            cr_assert_leq(positionOf(registry, blocks[block]).y - positionOf(registry, blocks[block - 1]).y, jtype::OBSTACLE_SIZE.y);
        }
    }
}

Test(common_spawn, zapper_has_a_node_at_each_end_and_a_beam_between)
{
    Registry registry;
    const std::vector<Entity> pieces = jtype::spawnZapper(registry, 0.2F, 0.6F);

    cr_assert_gt(pieces.size(), 2U);
    cr_assert(registry.getComponent<Obstacle>(pieces.front()).kind == ObstacleKind::ZapperNode);
    cr_assert(registry.getComponent<Obstacle>(pieces.back()).kind == ObstacleKind::ZapperNode);
    for (std::size_t piece = 1; piece + 1 < pieces.size(); ++piece) {
        cr_assert(registry.getComponent<Obstacle>(pieces[piece]).kind == ObstacleKind::ZapperBeam);
    }
    cr_assert_not(registry.hasComponent<jtype::Health>(pieces[1]));
}

Test(common_spawn, player_starts_with_the_lives_given)
{
    Registry registry;
    const Entity normal = jtype::spawnPlayer(registry, 0);
    const Entity easy = jtype::spawnPlayer(registry, 1, jtype::MAX_LIVES);

    cr_assert_eq(registry.getComponent<jtype::Player>(normal).lives, jtype::PLAYER_LIVES);
    cr_assert_eq(registry.getComponent<jtype::Player>(easy).lives, jtype::MAX_LIVES);
}
