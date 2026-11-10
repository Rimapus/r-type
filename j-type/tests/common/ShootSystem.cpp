/*
** EPITECH PROJECT, 2026
** j-type/tests/common/ShootSystem
** File description:
** ShootSystem unit tests
*/

#include <criterion/criterion.h>

#include <algorithm>
#include <cstdint>
#include <vector>

#include "jtype/common/Gameplay.hpp"
#include "jtype/common/Input.hpp"
#include "jtype/common/components/Missile.hpp"
#include "jtype/common/components/Player.hpp"
#include "jtype/common/systems/ShootSystem.hpp"
#include "younity/ecs/Registry.hpp"
#include "younity/ecs/components/Transform.hpp"
#include "younity/ecs/components/Velocity.hpp"

using jtype::Missile;
using jtype::Player;
using jtype::ShootSystem;
using younity::Vector2f;
using younity::ecs::Entity;
using younity::ecs::Registry;
using younity::ecs::Transform;

namespace {

// The ship of the player of slot 2, holding the given inputs
Entity createShip(Registry &registry, std::uint8_t inputs)
{
    const Entity ship = registry.createEntity();

    registry.addSystem<ShootSystem, Player, Transform>();
    registry.addComponent(ship, Transform{
                                    Vector2f{100.0F, 50.0F}
    });
    registry.addComponent(ship, Player{2, inputs});
    return ship;
}

std::vector<Entity> missiles(const Registry &registry)
{
    return registry.getEntitiesWith<Missile>();
}

} // namespace

Test(common_shoot_system, shoots_from_the_muzzle)
{
    Registry registry;

    createShip(registry, jtype::input::SHOOT);
    registry.updateSystems(jtype::SHOOT_COOLDOWN / 10.0F);
    cr_assert_eq(missiles(registry).size(), 1U);
    cr_assert(registry.getComponent<Transform>(missiles(registry)[0]).position == (Vector2f{100.0F, 50.0F} + jtype::MUZZLE_OFFSET));
}

Test(common_shoot_system, missile_belongs_to_the_player)
{
    Registry registry;

    createShip(registry, jtype::input::SHOOT);
    registry.updateSystems(0.01F);
    cr_assert(registry.getComponent<Missile>(missiles(registry)[0]).shooter == std::uint8_t{2});
}

Test(common_shoot_system, waits_for_the_cooldown)
{
    Registry registry;

    createShip(registry, jtype::input::SHOOT);
    // The first shot, then three quarters of the cooldown: too early for the second one
    registry.updateSystems(0.01F);
    registry.updateSystems(jtype::SHOOT_COOLDOWN * 0.75F);
    cr_assert_eq(missiles(registry).size(), 1U);
    registry.updateSystems(jtype::SHOOT_COOLDOWN * 0.5F);
    cr_assert_eq(missiles(registry).size(), 2U);
}

Test(common_shoot_system, does_not_shoot_without_the_input)
{
    Registry registry;

    createShip(registry, jtype::input::RIGHT);
    registry.updateSystems(1.0F);
    cr_assert(missiles(registry).empty());
}

Test(common_shoot_system, spread_fires_three_pellets_in_a_fan)
{
    Registry registry;
    const Entity ship = createShip(registry, jtype::input::SHOOT);

    registry.getComponent<Player>(ship).weapon = jtype::Weapon::Spread;
    registry.updateSystems(jtype::SHOOT_COOLDOWN / 10.0F);
    cr_assert_eq(missiles(registry).size(), 3U);

    float lowest = 0.0F;
    float highest = 0.0F;

    for (const Entity pellet : missiles(registry)) {
        const Missile &missile = registry.getComponent<Missile>(pellet);
        const Vector2f velocity = registry.getComponent<younity::ecs::Velocity>(pellet).value;

        cr_assert(missile.kind == jtype::MissileKind::Pellet);
        cr_assert_eq(*missile.shooter, 2);
        cr_assert_gt(velocity.x, 0.0F);
        lowest = std::min(lowest, velocity.y);
        highest = std::max(highest, velocity.y);
    }
    cr_assert_float_eq(lowest, -highest, 1e-3F);
    cr_assert_gt(highest, 0.0F);
}
