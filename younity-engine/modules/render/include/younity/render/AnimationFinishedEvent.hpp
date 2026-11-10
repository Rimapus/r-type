/*
** EPITECH PROJECT, 2026
** younity-engine/modules/render/include/younity/render/AnimationFinishedEvent
** File description:
** AnimationFinishedEvent header file
*/

#pragma once

#include "younity/ecs/Entity.hpp"

namespace younity::render {

/**
 * @brief Published by the AnimationSystem once, when a clip that does not loop has shown its last frame,
 *        e.g. to destroy an explosion once it is over.
 */
struct AnimationFinishedEvent
{
    ecs::Entity entity;
};

} // namespace younity::render
