/*
** EPITECH PROJECT, 2026
** j-climb/src/World
** File description:
** World header file
*/

#pragma once

#include <filesystem>

#include "younity/assets/ResourceManager.hpp"
#include "younity/core/EventBus.hpp"
#include "younity/ecs/ComponentRegistry.hpp"
#include "younity/ecs/Registry.hpp"
#include "younity/scripting/Host.hpp"

namespace jclimb {

/**
 * @brief The scene file the world starts from: it only holds the entities of rooms.lua and game.lua, the scripts
 *        build everything else.
 */
inline constexpr const char *SCENE_FILE = "scenes/game.json";

/**
 * @brief The entities of the game and the systems that run them, without a window: the whole game is written in the
 *        Lua scripts of the assets folder, the world only runs them, moves the entities and finds their collisions.
 */
class World
{
public:
    /**
     * @brief Register the components the scripts use, add the systems and load a scene file.
     *
     * @param resources The assets folder, with a loader for the textures; it must outlive the world
     * @param host What the scripts reach outside the entities: the actions, the audio, the game loop and the saves
     * @param scene The scene file, relative to the assets folder, e.g. a test room
     *
     * @throw std::runtime_error if the scene file cannot be loaded.
     */
    World(younity::assets::ResourceManager &resources, const younity::scripting::Host &host,
          const std::filesystem::path &scene = SCENE_FILE);

    /**
     * @brief Run one step: the scripts, then the movement of the entities with a speed, then the collisions, which
     *        the scripts get at the next step.
     *
     * @param step The duration of the step, in seconds
     */
    void update(float step);

    /**
     * @brief Get the entities, to draw them.
     *
     * @return The registry of the world.
     */
    [[nodiscard]] const younity::ecs::Registry &getRegistry(void) const noexcept;

private:
    younity::ecs::ComponentRegistry _components;
    younity::EventBus _bus;
    younity::ecs::Registry _registry;
};

} // namespace jclimb
