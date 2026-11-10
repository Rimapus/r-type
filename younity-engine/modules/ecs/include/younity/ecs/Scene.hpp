/*
** EPITECH PROJECT, 2026
** younity-engine/modules/ecs/include/younity/ecs/Scene
** File description:
** Scene header file
*/

#pragma once

#include "younity/core/EventBus.hpp"
#include "younity/ecs/Registry.hpp"

namespace younity::ecs {

/**
 * @brief A screen of the game, e.g. the menu or a level, with its own entities, systems and events.
 *        Its constructor sets it up, and everything it owns is destroyed with it.
 */
class Scene
{
public:
    virtual ~Scene(void) = default;

    /**
     * @brief Run one frame of the scene: all the systems of its registry, unless it is overridden.
     *
     * @param dt The time elapsed since the last frame, in seconds (delta time)
     */
    virtual void update(float dt);

    /**
     * @brief Get the registry owning the entities and the systems of the scene.
     *
     * @return The registry of the scene.
     */
    [[nodiscard]] Registry &getRegistry(void) noexcept;

    /**
     * @brief Get the event bus of the scene, destroyed with it: its systems can subscribe to it safely.
     *
     * @return The event bus of the scene.
     */
    [[nodiscard]] EventBus &getEventBus(void) noexcept;

private:
    EventBus _bus;
    Registry _registry;
};

} // namespace younity::ecs
