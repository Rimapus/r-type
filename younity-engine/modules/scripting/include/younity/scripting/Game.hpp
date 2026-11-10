/*
** EPITECH PROJECT, 2026
** younity-engine/modules/scripting/include/younity/scripting/Game
** File description:
** Game header file
*/

#pragma once

#include <functional>
#include <string>

namespace younity::scripting {

/**
 * @brief What the scripts ask the game loop, which owns the registry and the time.
 *        An empty function does nothing.
 */
struct Game
{
    /**
     * @brief Replaces the entities with the ones of a scene file, relative to the assets of the game.
     *        The scripts ask during a step and ecs::loadScene() throws during updateSystems(): the game loads the
     *        scene once the step is over.
     */
    std::function<void(const std::string &path)> loadScene = {};
    /** @brief Stops the game loop, e.g. with Application::stop(). */
    std::function<void(void)> quit = {};
    /** @brief Multiplies the duration of the next steps, 0 to freeze the game and 1 for the normal speed. */
    std::function<void(float scale)> setTimeScale = {};
};

} // namespace younity::scripting
