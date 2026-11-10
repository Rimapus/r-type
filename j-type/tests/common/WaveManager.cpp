/*
** EPITECH PROJECT, 2026
** j-type/tests/common/WaveManager
** File description:
** WaveManager unit tests
*/

#include <criterion/criterion.h>

#include <stdexcept>
#include <vector>

#include "jtype/common/Constants.hpp"
#include "jtype/common/Gameplay.hpp"
#include "jtype/common/WaveManager.hpp"
#include "jtype/common/components/Boss.hpp"
#include "jtype/common/components/Enemy.hpp"
#include "jtype/common/components/Health.hpp"
#include "jtype/common/components/Obstacle.hpp"
#include "younity/ecs/Registry.hpp"
#include "younity/ecs/components/Transform.hpp"

using jtype::Boss;
using jtype::BossPart;
using jtype::BossType;
using jtype::Enemy;
using jtype::EnemyType;
using jtype::Formation;
using jtype::Health;
using jtype::MovementPattern;
using jtype::Obstacle;
using jtype::Wave;
using jtype::WaveGroup;
using jtype::WaveManager;
using jtype::WaveObstacle;
using younity::Vector2f;
using younity::ecs::Entity;
using younity::ecs::Registry;
using younity::ecs::Transform;

namespace {

// After 1 s of calm, 3 Jellies flying straight, one every 0.5 s
const std::vector<Wave> ONE_WAVE{
    Wave{1.0F, {WaveGroup{EnemyType::Jelly, MovementPattern::Straight, 3, 0.5F}}}
};

std::size_t countBydos(const Registry &registry)
{
    return registry.getEntitiesWith<Enemy>().size();
}

// Destroy the Bydos, as the players or the left edge would
void clearBydos(Registry &registry)
{
    for (const Entity bydo : registry.getEntitiesWith<Enemy>()) {
        registry.destroyEntity(bydo);
    }
}

// The first wave without a pause, then the boss after 1 s of calm
const std::vector<Wave> BOSS_WAVES{
    Wave{0.0F, {WaveGroup{EnemyType::Blobling, MovementPattern::Straight, 1, 0.0F}}},
    Wave{1.0F, {}, BossType::Incubator},
};

// Destroy the boss, as the players would
void clearBoss(Registry &registry)
{
    for (const Entity part : registry.getEntitiesWith<Boss>()) {
        registry.destroyEntity(part);
    }
}

// Play a wave of one group with no pause and no delay, and give where its Bydos appeared
std::vector<Vector2f> spawnGroup(Formation formation, std::size_t count, float height)
{
    Registry registry;
    const std::vector<Wave> wave{
        Wave{0.0F, {WaveGroup{EnemyType::Blobling, MovementPattern::Straight, count, 0.0F, formation, height}}}
    };
    WaveManager waves(wave, 1);
    std::vector<Vector2f> positions;

    waves.update(registry, 0.0F, 1);
    for (const Entity bydo : registry.getEntitiesWith<Enemy>()) {
        positions.push_back(registry.getComponent<Transform>(bydo).position);
    }
    return positions;
}

// The height of a Bydo of a group at a height from 0 to 1, as the WaveManager computes it: between the ceiling and the
// floor
float heightOf(float height)
{
    const float top = jtype::TERRAIN_HEIGHT + jtype::BYDO_ORIGIN.y;
    const float bottom = jtype::WORLD_HEIGHT - jtype::TERRAIN_HEIGHT - jtype::BYDO_SIZE.y + jtype::BYDO_ORIGIN.y;

    return top + (height * (bottom - top));
}

} // namespace

Test(common_wave_manager, waits_for_the_pause)
{
    Registry registry;
    WaveManager waves(ONE_WAVE, 1);

    waves.update(registry, 0.9F, 1);
    cr_assert_eq(countBydos(registry), 0U);
    cr_assert_eq(waves.getStartedWaves(), 0U);
    waves.update(registry, 0.2F, 1);
    cr_assert_eq(countBydos(registry), 1U);
    cr_assert_eq(waves.getStartedWaves(), 1U);
}

Test(common_wave_manager, brings_the_group_one_after_the_other)
{
    Registry registry;
    WaveManager waves(ONE_WAVE, 1);

    waves.update(registry, 1.0F, 1);
    waves.update(registry, 0.5F, 1);
    cr_assert_eq(countBydos(registry), 2U);
    waves.update(registry, 0.5F, 1);
    waves.update(registry, 0.5F, 1);
    cr_assert_eq(countBydos(registry), 3U);
}

Test(common_wave_manager, uses_the_type_and_the_pattern_of_the_group)
{
    Registry registry;
    WaveManager waves(ONE_WAVE, 1);

    waves.update(registry, 1.0F, 1);

    const Enemy &enemy = registry.getComponent<Enemy>(registry.getEntitiesWith<Enemy>()[0]);

    cr_assert(enemy.type == EnemyType::Jelly);
    cr_assert(enemy.pattern == MovementPattern::Straight);
}

Test(common_wave_manager, more_players_bring_more_bydos)
{
    Registry registry;
    WaveManager waves(ONE_WAVE, 1);

    // 3 Bydos, and a quarter of them for each of the 2 other players: 1 more
    waves.update(registry, 10.0F, 3);
    cr_assert_eq(countBydos(registry), 4U);
}

Test(common_wave_manager, waits_for_the_bydos_of_the_wave_before_the_pause)
{
    Registry registry;
    WaveManager waves(ONE_WAVE, 1);

    // The whole wave is out, but its Bydos are still there: the next wave waits
    waves.update(registry, 2.0F, 1);
    waves.update(registry, 10.0F, 1);
    cr_assert_eq(waves.getStartedWaves(), 1U);
    cr_assert_eq(countBydos(registry), 3U);
    // Once they are gone, the pause of the next wave starts
    clearBydos(registry);
    waves.update(registry, 0.9F, 1);
    cr_assert_eq(waves.getStartedWaves(), 1U);
    waves.update(registry, 0.2F, 1);
    cr_assert_eq(waves.getStartedWaves(), 2U);
}

Test(common_wave_manager, plays_the_waves_again_faster)
{
    Registry registry;
    WaveManager waves(ONE_WAVE, 1);

    // The first round: the pause, then 3 Bydos 0.5 s apart, destroyed
    waves.update(registry, 1.0F, 1);
    waves.update(registry, 1.0F, 1);
    clearBydos(registry);
    // The pause of the second round, then its first Bydo
    waves.update(registry, 1.0F, 1);
    cr_assert_eq(waves.getStartedWaves(), 2U);
    cr_assert_eq(countBydos(registry), 1U);
    // 0.5 s is enough for the next one: its delay is now 0.5 * WAVE_SPEEDUP
    waves.update(registry, 0.5F * jtype::WAVE_SPEEDUP, 1);
    cr_assert_eq(countBydos(registry), 2U);
}

Test(common_wave_manager, line_brings_the_bydos_at_the_height_of_the_group)
{
    for (const Vector2f &position : spawnGroup(Formation::Line, 3, 0.25F)) {
        cr_assert_float_eq(position.y, heightOf(0.25F), 1e-3F);
        cr_assert_gt(position.x, jtype::WORLD_WIDTH);
    }
}

Test(common_wave_manager, column_stacks_the_bydos_around_the_height)
{
    const std::vector<Vector2f> positions = spawnGroup(Formation::Column, 3, 0.5F);

    cr_assert_eq(positions.size(), 3U);
    cr_assert_float_eq(positions[0].y, heightOf(0.5F) - jtype::FORMATION_GAP, 1e-3F);
    cr_assert_float_eq(positions[1].y, heightOf(0.5F), 1e-3F);
    cr_assert_float_eq(positions[2].y, heightOf(0.5F) + jtype::FORMATION_GAP, 1e-3F);
    cr_assert_float_eq(positions[0].x, positions[2].x, 1e-3F);
}

Test(common_wave_manager, v_puts_the_bydos_by_pairs_behind_the_first)
{
    const std::vector<Vector2f> positions = spawnGroup(Formation::V, 3, 0.5F);
    const float gap = jtype::FORMATION_GAP;

    cr_assert_eq(positions.size(), 3U);
    cr_assert_float_eq(positions[0].y, heightOf(0.5F), 1e-3F);
    // One step back, one above and one below
    cr_assert(positions[1] == (Vector2f{positions[0].x + gap, positions[0].y - gap}));
    cr_assert(positions[2] == (Vector2f{positions[0].x + gap, positions[0].y + gap}));
}

Test(common_wave_manager, formation_stays_inside_the_screen)
{
    for (const Vector2f &position : spawnGroup(Formation::Column, 5, 0.0F)) {
        cr_assert_geq(position.y, heightOf(0.0F));
    }
}

Test(common_wave_manager, last_bydo_of_a_drop_group_carries_a_power_up)
{
    Registry registry;
    const std::vector<Wave> wave{
        Wave{0.0F, {WaveGroup{EnemyType::Blobling, MovementPattern::Straight, 3, 0.0F, Formation::Line, 0.5F, true}}}
    };
    WaveManager waves(wave, 1);

    waves.update(registry, 0.0F, 1);

    const std::vector<Entity> bydos = registry.getEntitiesWith<Enemy>();

    cr_assert_eq(bydos.size(), 3U);
    cr_assert_not(registry.getComponent<Enemy>(bydos[0]).drop.has_value());
    cr_assert_not(registry.getComponent<Enemy>(bydos[1]).drop.has_value());
    cr_assert(registry.getComponent<Enemy>(bydos[2]).drop.has_value());
}

Test(common_wave_manager, brings_the_obstacles_with_their_wave)
{
    Registry registry;
    std::vector<Wave> wave{
        Wave{0.0F, {}}
    };

    wave[0].obstacles = {
        WaveObstacle{jtype::ObstacleKind::Block,      0.0F, 0.0F},
        WaveObstacle{jtype::ObstacleKind::ZapperNode, 0.0F, 1.0F},
    };

    WaveManager waves(wave, 1);

    waves.update(registry, 0.0F, 1);
    // One block, and a zapper from the ceiling to the floor
    cr_assert_gt(registry.getEntitiesWith<Obstacle>().size(), 3U);
    cr_assert_eq(waves.getStartedWaves(), 1U);
}

Test(common_wave_manager, brings_the_boss_with_its_wave)
{
    Registry registry;
    WaveManager waves(BOSS_WAVES, 1);

    waves.update(registry, 0.0F, 3);
    clearBydos(registry);
    waves.update(registry, 1.0F, 3);
    cr_assert_eq(registry.getEntitiesWith<Boss>().size(), 2U);
    // The eye has the health of one player, and more for each of the 2 other players
    for (const Entity part : registry.getEntitiesWith<Boss>()) {
        if (registry.getComponent<Boss>(part).part == BossPart::Eye) {
            cr_assert_eq(registry.getComponent<Health>(part).max, jtype::BOSS_HEALTH + (2 * jtype::BOSS_HEALTH_PER_PLAYER));
        }
    }
}

Test(common_wave_manager, beating_the_boss_wins_the_game)
{
    Registry registry;
    WaveManager waves(BOSS_WAVES, 1);

    waves.update(registry, 0.0F, 1);
    clearBydos(registry);
    waves.update(registry, 1.0F, 1);
    waves.update(registry, 10.0F, 1);
    cr_assert_not(waves.isWon());
    clearBoss(registry);
    waves.update(registry, 0.1F, 1);
    cr_assert(waves.isWon());
    // No wave comes after the boss
    waves.update(registry, 10.0F, 1);
    cr_assert_eq(waves.getStartedWaves(), 2U);
    cr_assert(registry.getEntitiesWith<Enemy>().empty());
}

Test(common_wave_manager, same_seed_gives_the_same_heights)
{
    Registry first;
    Registry second;
    WaveManager firstWaves(ONE_WAVE, 9);
    WaveManager secondWaves(ONE_WAVE, 9);

    firstWaves.update(first, 5.0F, 1);
    secondWaves.update(second, 5.0F, 1);
    for (std::size_t i = 0; i < 3; ++i) {
        const Vector2f a = first.getComponent<Transform>(first.getEntitiesWith<Enemy>()[i]).position;
        const Vector2f b = second.getComponent<Transform>(second.getEntitiesWith<Enemy>()[i]).position;

        cr_assert(a == b);
    }
}

Test(common_wave_manager, needs_a_wave)
{
    cr_assert_throw(WaveManager({}, 1), std::invalid_argument);
}
