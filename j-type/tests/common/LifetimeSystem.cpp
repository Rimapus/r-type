/*
** EPITECH PROJECT, 2026
** j-type/tests/common/LifetimeSystem
** File description:
** LifetimeSystem and InvulnerabilitySystem unit tests
*/

#include <criterion/criterion.h>

#include "jtype/common/Gameplay.hpp"
#include "jtype/common/Spawn.hpp"
#include "jtype/common/components/Lifetime.hpp"
#include "jtype/common/components/Player.hpp"
#include "jtype/common/systems/InvulnerabilitySystem.hpp"
#include "jtype/common/systems/LifetimeSystem.hpp"
#include "younity/ecs/Registry.hpp"

using jtype::InvulnerabilitySystem;
using jtype::Lifetime;
using jtype::LifetimeSystem;
using jtype::Player;
using younity::Vector2f;
using younity::ecs::Entity;
using younity::ecs::Registry;

Test(common_lifetime_system, explosion_lasts_its_duration)
{
    Registry registry;

    registry.addSystem<LifetimeSystem, Lifetime>();

    const Entity explosion = jtype::spawnExplosion(registry, Vector2f{});

    registry.updateSystems(jtype::EXPLOSION_DURATION / 2.0F);
    cr_assert(registry.isAlive(explosion));
    registry.updateSystems(jtype::EXPLOSION_DURATION / 2.0F);
    cr_assert_not(registry.isAlive(explosion));
}

Test(common_invulnerability_system, counts_down_to_zero)
{
    Registry registry;

    registry.addSystem<InvulnerabilitySystem, Player>();

    const Entity ship = jtype::spawnPlayer(registry, 0);

    registry.getComponent<Player>(ship).invulnerable = 1.0F;
    registry.updateSystems(0.75F);
    cr_assert_float_eq(registry.getComponent<Player>(ship).invulnerable, 0.25F, 1e-6F);
    registry.updateSystems(0.75F);
    cr_assert_float_eq(registry.getComponent<Player>(ship).invulnerable, 0.0F, 1e-6F);
}
