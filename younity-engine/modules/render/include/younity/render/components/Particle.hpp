/*
** EPITECH PROJECT, 2026
** younity-engine/modules/render/include/younity/render/components/Particle
** File description:
** Particle header file
*/

#pragma once

#include "younity/core/Vector2.hpp"

namespace younity::render {

/**
 * @brief A particle spawned by a ParticleEmitter: the ParticleSystem moves it and destroys it at the end of its life.
 */
struct Particle
{
    // Pixels per second
    Vector2f velocity{};
    // Seconds left to live
    float remaining = 0.0F;
};

} // namespace younity::render
