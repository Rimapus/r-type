/*
** EPITECH PROJECT, 2026
** younity-engine/modules/scripting/include/younity/scripting/systems/ScriptSystem
** File description:
** ScriptSystem header file
*/

#pragma once

#include <filesystem>
#include <memory>
#include <vector>

#include "younity/core/EventBus.hpp"
#include "younity/ecs/ComponentRegistry.hpp"
#include "younity/ecs/Entity.hpp"
#include "younity/ecs/System.hpp"
#include "younity/scripting/Actions.hpp"
#include "younity/scripting/Host.hpp"

namespace younity::scripting {

/**
 * @brief Runs the Lua script of each entity at each step.
 *        Add it with registry.addSystem<ScriptSystem, Script>(components, bus, actions, "assets").
 *
 * A script is a Lua file that returns a table with three optional functions:
 * start(entity), called once before the first update, update(entity, dt), called at each step,
 * and onCollision(entity, other), called before update for each physics::CollisionEvent of the entity
 * published since the last step.
 * The file runs once per entity, so each entity has its own table.
 * A script that fails to load or raises an error is logged once and stops for its entity.
 * A script whose file is saved again starts again at the next step, with a new table: a fixed script runs again.
 * The scripts run in one Lua interpreter with the base, coroutine, math, string, table and utf8 libraries:
 * no io, os or package, and no dofile or loadfile, but a require loading the modules of the folder of the scripts.
 * The ecs table lets a script create and destroy entities, and read and write the components registered
 * in the ComponentRegistry by their name, e.g. ecs.get(entity, "Transform").position.x.
 * The input table reads and binds the actions given by the game, e.g. input.isPressed("shoot"),
 * the audio table plays sounds and music, the game table changes scene, quits and sets the time scale,
 * the storage table saves tables to JSON files, and the log table writes to the logger of the module,
 * e.g. log.info("spawned"). The input, audio and game tables call the functions of the Host given by the game.
 */
class ScriptSystem : public ecs::System
{
public:
    /**
     * @brief Create the system and its Lua interpreter, for a game that gives its scripts its actions only:
     *        the audio and game tables do nothing and the storage table saves nothing.
     *
     * @param components The component registry of the game: the scripts reach the components registered in it,
     *        it must outlive the system
     * @param bus The event bus the CollisionSystem publishes on
     * @param actions The actions the scripts read, copied: what their functions capture must outlive the system
     * @param root The folder the paths of the scripts are relative to, the current folder by default
     */
    ScriptSystem(const ecs::ComponentRegistry &components, EventBus &bus, const Actions &actions,
                 std::filesystem::path root = {});

    /**
     * @brief Create the system and its Lua interpreter, with everything the game gives its scripts.
     *
     * @param components The component registry of the game: the scripts reach the components registered in it,
     *        it must outlive the system
     * @param bus The event bus the CollisionSystem publishes on
     * @param host The input, audio, game loop and saves the scripts reach, copied: what their functions capture
     *        must outlive the system
     * @param root The folder the paths of the scripts and the names of the modules are relative to, the current
     *        folder by default
     */
    ScriptSystem(const ecs::ComponentRegistry &components, EventBus &bus, const Host &host,
                 std::filesystem::path root = {});

    /**
     * @brief Destroy the system, its interpreter and the state of every script.
     */
    ~ScriptSystem(void) override;

    ScriptSystem(const ScriptSystem &) = delete;
    ScriptSystem &operator=(const ScriptSystem &) = delete;
    ScriptSystem(ScriptSystem &&) = delete;
    ScriptSystem &operator=(ScriptSystem &&) = delete;

    /**
     * @brief Start the scripts of the new entities, then call the update of each script.
     *        A script whose path changed or whose file was saved again starts again,
     *        and the state of the entities that lost their script is freed.
     *
     * @param registry The registry owning the entities
     * @param entities The entities having a Script
     * @param dt The time elapsed since the last frame, in seconds (delta time)
     */
    void update(ecs::Registry &registry, const std::vector<ecs::Entity> &entities, float dt) override;

    /**
     * @brief Tell if the script of an entity runs: it started during an update and raised no error.
     *
     * @param entity The entity
     *
     * @return true if its script runs, false if it failed or has not started.
     */
    [[nodiscard]] bool isRunning(ecs::Entity entity) const;

private:
    struct Impl;

    std::unique_ptr<Impl> _impl;
};

} // namespace younity::scripting
