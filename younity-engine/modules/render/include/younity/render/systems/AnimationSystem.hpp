/*
** EPITECH PROJECT, 2026
** younity-engine/modules/render/include/younity/render/systems/AnimationSystem
** File description:
** AnimationSystem header file
*/

#pragma once

#include <vector>

#include "younity/core/EventBus.hpp"
#include "younity/ecs/Entity.hpp"
#include "younity/ecs/System.hpp"

namespace younity::render {

/**
 * @brief Moves the animations forward, at each fixed step: they run at the same speed whatever the frame rate.
 *        Add it with registry.addSystem<AnimationSystem, Animation>(bus).
 */
class AnimationSystem : public ecs::System
{
public:
    /**
     * @brief Create the system.
     *
     * @param bus The event bus the ends of animation are published on
     */
    explicit AnimationSystem(EventBus &bus);

    /**
     * @brief Move each animation forward by the delta time, and publish an AnimationFinishedEvent
     *        for each clip that does not loop and has just ended.
     *
     * @param registry The registry owning the entities
     * @param entities The entities having an Animation
     * @param dt The time elapsed since the last step, in seconds (delta time)
     */
    void update(ecs::Registry &registry, const std::vector<ecs::Entity> &entities, float dt) override;

private:
    EventBus &_bus;
};

} // namespace younity::render
