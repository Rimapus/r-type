/*
** EPITECH PROJECT, 2026
** younity-engine/modules/render/include/younity/render/components/ParticleEmitter
** File description:
** ParticleEmitter header file
*/

#pragma once

#include "younity/render/components/Sprite.hpp"

namespace younity::render {

/**
 * @brief Spawns short-lived sprites, the particles, at the Transform of its entity, e.g. for explosions,
 *        engine trails and sparks. The ParticleSystem spawns, moves and destroys them.
 */
struct ParticleEmitter
{
    // The sprite of each particle
    Sprite sprite;
    // Particles spawned per second while emitting is true
    float rate = 10.0F;
    // Particles spawned at once at the next step, then set back to 0, e.g. 30 for an explosion
    unsigned int burst = 0;
    // false stops the flow of particles, not the burst; the particles already spawned live on
    bool emitting = true;
    // Seconds a particle lives
    float lifetime = 1.0F;
    // Pixels per second of a particle
    float speed = 50.0F;
    // Each particle gets a speed between speed - speedVariation and speed + speedVariation
    float speedVariation = 0.0F;
    // The direction of the particles in degrees, clockwise from the right like the rotation of a Transform
    float direction = 0.0F;
    // The angle of the cone the particles go in, in degrees, centered on direction: 360 for every direction
    float spread = 360.0F;
    // The part of a particle the last steps did not spawn yet, below 1
    float pending = 0.0F;
};

} // namespace younity::render
