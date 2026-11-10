/*
** EPITECH PROJECT, 2026
** younity-engine/modules/render/src/systems/ParticleSystem
** File description:
** ParticleSystem source file
*/

#include "younity/render/systems/ParticleSystem.hpp"
#include "younity/ecs/Registry.hpp"
#include "younity/ecs/components/Transform.hpp"
#include "younity/render/components/Particle.hpp"
#include "younity/render/components/ParticleEmitter.hpp"

#include <algorithm>
#include <cmath>
#include <numbers>

namespace younity::render {

namespace {

/**
 * @brief Read a field of an emitter that cannot be negative, e.g. written by hand in a scene file.
 *
 * @param value The field
 *
 * @return The value, or 0 if it is negative or not a finite number.
 */
float nonNegative(float value)
{
    return std::isfinite(value) && value > 0.0F ? value : 0.0F;
}

/**
 * @brief Read a field of an emitter that can be negative.
 *
 * @param value The field
 *
 * @return The value, or 0 if it is not a finite number.
 */
float finite(float value)
{
    return std::isfinite(value) ? value : 0.0F;
}

/**
 * @brief Take the particles an emitter spawns in this step out of its burst and its flow.
 *
 * @param emitter The emitter, its burst emptied and its pending part updated
 * @param dt The time elapsed since the last step, in seconds
 *
 * @return The number of particles to spawn, at most MAX_PARTICLES_PER_STEP.
 */
unsigned int takeParticles(ParticleEmitter &emitter, float dt)
{
    float count = static_cast<float>(emitter.burst);

    emitter.burst = 0;
    if (emitter.emitting) {
        emitter.pending = nonNegative(emitter.pending) + (nonNegative(emitter.rate) * nonNegative(dt));
        count += std::floor(emitter.pending);
        emitter.pending -= std::floor(emitter.pending);
    }
    return static_cast<unsigned int>(std::min(count, static_cast<float>(MAX_PARTICLES_PER_STEP)));
}

} // namespace

ParticleSystem::ParticleSystem(std::uint32_t seed)
    : _random(seed)
{
}

void ParticleSystem::update(ecs::Registry &registry, const std::vector<ecs::Entity> &entities, float dt)
{
    for (const ecs::Entity entity : registry.getEntitiesWith<Particle, ecs::Transform>()) {
        Particle &particle = registry.getComponent<Particle>(entity);

        particle.remaining -= dt;
        // Not "<= 0": a NaN lifetime must end too
        if (!(particle.remaining > 0.0F)) {
            registry.destroyEntity(entity);
            continue;
        }
        registry.getComponent<ecs::Transform>(entity).position += particle.velocity * dt;
    }
    for (const ecs::Entity entity : entities) {
        const unsigned int count = takeParticles(registry.getComponent<ParticleEmitter>(entity), dt);

        if (count == 0) {
            continue;
        }
        // Copied: spawning entities may move the components of the registry
        const ParticleEmitter emitter = registry.getComponent<ParticleEmitter>(entity);
        const Vector2f position = registry.getComponent<ecs::Transform>(entity).position;
        const float direction = finite(emitter.direction);
        const float spread = std::abs(finite(emitter.spread));
        const float speed = finite(emitter.speed);
        const float variation = std::abs(finite(emitter.speedVariation));
        std::uniform_real_distribution<float> angles(direction - (spread / 2.0F), direction + (spread / 2.0F));
        std::uniform_real_distribution<float> speeds(speed - variation, speed + variation);

        for (unsigned int spawned = 0; spawned < count; ++spawned) {
            const float angle = angles(this->_random) * std::numbers::pi_v<float> / 180.0F;
            const Vector2f velocity = Vector2f{std::cos(angle), std::sin(angle)} * speeds(this->_random);
            const ecs::Entity particle = registry.createEntity();

            registry.addComponent(particle, ecs::Transform{position});
            registry.addComponent(particle, emitter.sprite);
            registry.addComponent(particle, Particle{velocity, emitter.lifetime});
        }
    }
}

} // namespace younity::render
