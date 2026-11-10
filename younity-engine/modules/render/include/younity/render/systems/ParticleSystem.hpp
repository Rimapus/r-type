/*
** EPITECH PROJECT, 2026
** younity-engine/modules/render/include/younity/render/systems/ParticleSystem
** File description:
** ParticleSystem header file
*/

#pragma once

#include <cstdint>
#include <random>
#include <vector>

#include "younity/ecs/Entity.hpp"
#include "younity/ecs/System.hpp"

namespace younity::render {

/**
 * @brief The most particles one emitter spawns in one step, so a wrong rate or burst cannot fill the memory.
 */
inline constexpr unsigned int MAX_PARTICLES_PER_STEP = 1000;

/**
 * @brief Spawns the particles of the emitters, moves them and destroys them at the end of their life,
 *        at each fixed step. A particle is an entity with a Transform, a Sprite and a Particle.
 *        Add it with registry.addSystem<ParticleSystem, ParticleEmitter, ecs::Transform>().
 */
class ParticleSystem : public ecs::System
{
public:
    /**
     * @brief Create the system.
     *
     * @param seed The seed of the random directions and speeds, e.g. a fixed one in the tests
     */
    explicit ParticleSystem(std::uint32_t seed = std::random_device{}());

    /**
     * @brief Move the particles and destroy the ones whose life ended, then spawn the new particles of each emitter
     *        at its position, MAX_PARTICLES_PER_STEP at most. A particle spawned in a step moves from the next one.
     *        A negative or non-finite rate counts as 0, and a negative speedVariation or spread as its opposite.
     *
     * @param registry The registry owning the entities
     * @param entities The entities having a ParticleEmitter and a Transform
     * @param dt The time elapsed since the last step, in seconds (delta time)
     */
    void update(ecs::Registry &registry, const std::vector<ecs::Entity> &entities, float dt) override;

private:
    std::mt19937 _random;
};

} // namespace younity::render
