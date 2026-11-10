/*
** EPITECH PROJECT, 2026
** younity-engine/tests/render/ParticleSystem
** File description:
** ParticleSystem unit tests
*/

#include <criterion/criterion.h>

#include <limits>

#include "younity/ecs/Registry.hpp"
#include "younity/ecs/components/Transform.hpp"
#include "younity/render/components/Particle.hpp"
#include "younity/render/components/ParticleEmitter.hpp"
#include "younity/render/components/Sprite.hpp"
#include "younity/render/systems/ParticleSystem.hpp"

using younity::Vector2f;
using younity::ecs::Entity;
using younity::ecs::Registry;
using younity::ecs::Transform;
using younity::render::MAX_PARTICLES_PER_STEP;
using younity::render::Particle;
using younity::render::ParticleEmitter;
using younity::render::ParticleSystem;

namespace {

// An emitter at (10, 20), with a fixed seed for the random directions
Entity createEmitter(Registry &registry, const ParticleEmitter &emitter)
{
    const Entity entity = registry.createEntity();

    registry.addSystem<ParticleSystem, ParticleEmitter, Transform>(42U);
    registry.addComponent(entity, Transform{
                                      Vector2f{10.0F, 20.0F}
    });
    registry.addComponent(entity, emitter);
    return entity;
}

std::size_t countParticles(const Registry &registry)
{
    return registry.getEntitiesWith<Particle>().size();
}

} // namespace

Test(render_particle_system, spawns_the_rate_of_particles)
{
    Registry registry;

    createEmitter(registry, ParticleEmitter{.sprite = {}, .rate = 10.0F});
    registry.updateSystems(0.5F);
    cr_assert_eq(countParticles(registry), 5U);
}

Test(render_particle_system, keeps_the_part_of_a_particle_for_the_next_steps)
{
    Registry registry;

    createEmitter(registry, ParticleEmitter{.sprite = {}, .rate = 3.0F, .lifetime = 10.0F});
    for (int step = 0; step < 4; ++step) {
        registry.updateSystems(0.25F);
    }
    cr_assert_eq(countParticles(registry), 3U);
}

Test(render_particle_system, spawns_a_burst_once)
{
    Registry registry;
    const Entity emitter = createEmitter(registry, ParticleEmitter{.sprite = {}, .burst = 20, .emitting = false});

    registry.updateSystems(0.1F);
    registry.updateSystems(0.1F);
    cr_assert_eq(countParticles(registry), 20U);
    cr_assert_eq(registry.getComponent<ParticleEmitter>(emitter).burst, 0U);
}

Test(render_particle_system, spawns_at_the_emitter_with_its_sprite)
{
    Registry registry;
    ParticleEmitter emitter{.sprite = {}, .burst = 1, .emitting = false};

    emitter.sprite.layer = 3;
    createEmitter(registry, emitter);
    registry.updateSystems(0.1F);

    const Entity particle = registry.getEntitiesWith<Particle>()[0];

    cr_assert(registry.getComponent<Transform>(particle).position == (Vector2f{10.0F, 20.0F}));
    cr_assert_eq(registry.getComponent<younity::render::Sprite>(particle).layer, 3);
    cr_assert_float_eq(registry.getComponent<Particle>(particle).remaining, 1.0F, 1e-6F);
}

Test(render_particle_system, moves_the_particles_in_the_direction)
{
    Registry registry;

    // Straight down, 10 pixels per second
    createEmitter(registry, ParticleEmitter{.sprite = {}, .burst = 1, .emitting = false, .speed = 10.0F, .direction = 90.0F, .spread = 0.0F});
    registry.updateSystems(0.1F);
    registry.updateSystems(0.5F);

    const Vector2f position = registry.getComponent<Transform>(registry.getEntitiesWith<Particle>()[0]).position;

    cr_assert_float_eq(position.x, 10.0F, 1e-4F);
    cr_assert_float_eq(position.y, 25.0F, 1e-4F);
}

Test(render_particle_system, destroys_the_particles_at_the_end_of_their_life)
{
    Registry registry;

    createEmitter(registry, ParticleEmitter{.sprite = {}, .burst = 4, .emitting = false, .lifetime = 1.0F});
    registry.updateSystems(0.6F);
    registry.updateSystems(0.6F);
    cr_assert_eq(countParticles(registry), 4U);
    registry.updateSystems(0.6F);
    cr_assert_eq(countParticles(registry), 0U);
}

Test(render_particle_system, stopped_emitter_spawns_nothing)
{
    Registry registry;

    createEmitter(registry, ParticleEmitter{.sprite = {}, .rate = 100.0F, .emitting = false});
    registry.updateSystems(1.0F);
    cr_assert_eq(countParticles(registry), 0U);
}

Test(render_particle_system, spawns_at_most_the_maximum_per_step)
{
    Registry registry;

    createEmitter(registry, ParticleEmitter{.sprite = {}, .rate = 1e9F, .burst = 4000000000U});
    registry.updateSystems(1.0F);
    cr_assert_eq(countParticles(registry), MAX_PARTICLES_PER_STEP);
}

Test(render_particle_system, negative_rate_spawns_nothing)
{
    Registry registry;
    const Entity emitter = createEmitter(registry, ParticleEmitter{.sprite = {}, .rate = -10.0F});

    registry.updateSystems(1.0F);
    cr_assert_eq(countParticles(registry), 0U);
    cr_assert_float_eq(registry.getComponent<ParticleEmitter>(emitter).pending, 0.0F, 1e-6F);
}

Test(render_particle_system, negative_variation_and_spread_are_used_as_positive)
{
    Registry registry;

    createEmitter(registry, ParticleEmitter{.sprite = {}, .burst = 10, .emitting = false, .speedVariation = -5.0F, .spread = -90.0F});
    registry.updateSystems(0.1F);
    cr_assert_eq(countParticles(registry), 10U);
}

Test(render_particle_system, particle_without_a_valid_lifetime_ends)
{
    Registry registry;

    createEmitter(registry, ParticleEmitter{.sprite = {}, .burst = 2, .emitting = false, .lifetime = std::numeric_limits<float>::quiet_NaN()});
    registry.updateSystems(0.1F);
    registry.updateSystems(0.1F);
    cr_assert_eq(countParticles(registry), 0U);
}
