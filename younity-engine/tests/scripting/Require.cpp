/*
** EPITECH PROJECT, 2026
** younity-engine/tests/scripting/Require
** File description:
** Require unit tests
*/

#include <criterion/criterion.h>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <string>

#include "younity/core/EventBus.hpp"
#include "younity/ecs/ComponentRegistry.hpp"
#include "younity/ecs/Registry.hpp"
#include "younity/scripting/Actions.hpp"
#include "younity/scripting/components/Script.hpp"
#include "younity/scripting/systems/ScriptSystem.hpp"

using younity::EventBus;
using younity::ecs::ComponentRegistry;
using younity::ecs::Entity;
using younity::ecs::Registry;
using younity::scripting::Actions;
using younity::scripting::Script;
using younity::scripting::ScriptSystem;

namespace {

const std::filesystem::path FIXTURES = std::filesystem::path(YOUNITY_TESTS_DIR) / "scripting" / "fixtures";

// Runs a script of the fixtures, then tells if it still runs
bool runs(const std::string &fixture)
{
    const ComponentRegistry components;
    EventBus bus;
    Registry registry;
    const ScriptSystem &scripts = registry.addSystem<ScriptSystem, Script>(components, bus, Actions{}, FIXTURES);
    const Entity entity = registry.createEntity();

    registry.addComponent(entity, Script{fixture});
    registry.updateSystems(0.5F);
    return scripts.isRunning(entity);
}

// Save a file, written after the previous save even on a file system that keeps the time in whole seconds
void save(const std::filesystem::path &file, const std::string &code)
{
    const bool existed = std::filesystem::exists(file);
    const std::filesystem::file_time_type previous = existed ? std::filesystem::last_write_time(file) : std::filesystem::file_time_type{};

    std::ofstream(file) << code;
    if (existed) {
        std::filesystem::last_write_time(file, previous + std::chrono::seconds(2));
    }
}

// Requires the rooms as it starts, and fails at its second update: it runs only if it started again in between
const char *const REQUIRES_THE_ROOMS = "local rooms = require('rooms')\n"
                                       "local Script = { updates = 0 }\n"
                                       "function Script.update(entity, dt)\n"
                                       "    Script.updates = Script.updates + 1\n"
                                       "    if Script.updates > 1 then error('updated twice') end\n"
                                       "end\n"
                                       "return Script\n";

} // namespace

Test(scripting_require, loads_a_module_once)
{
    cr_assert(runs("requires_a_module.lua"));
}

Test(scripting_require, missing_module_stops_the_script)
{
    cr_assert_not(runs("requires_a_missing_module.lua"));
}

Test(scripting_require, name_with_a_path_stops_the_script)
{
    cr_assert_not(runs("requires_with_a_wrong_name.lua"));
}

Test(scripting_require, module_requiring_itself_stops_the_script)
{
    cr_assert_not(runs("requires_a_loop.lua"));
}

Test(scripting_require, saving_a_module_starts_its_scripts_again)
{
    const std::filesystem::path folder = std::filesystem::temp_directory_path() / "younity_require_saved";
    const ComponentRegistry components;
    EventBus bus;
    Registry registry;

    std::filesystem::remove_all(folder);
    std::filesystem::create_directories(folder);
    save(folder / "rooms.lua", "return { count = 1 }\n");
    save(folder / "script.lua", REQUIRES_THE_ROOMS);

    const ScriptSystem &scripts = registry.addSystem<ScriptSystem, Script>(components, bus, Actions{}, folder);
    const Entity entity = registry.createEntity();

    registry.addComponent(entity, Script{"script.lua"});
    registry.updateSystems(0.5F);
    save(folder / "rooms.lua", "return { count = 2 }\n");
    registry.updateSystems(0.5F);
    cr_assert(scripts.isRunning(entity));
}

Test(scripting_require, unsaved_module_keeps_its_scripts_running)
{
    const std::filesystem::path folder = std::filesystem::temp_directory_path() / "younity_require_unsaved";
    const ComponentRegistry components;
    EventBus bus;
    Registry registry;

    std::filesystem::remove_all(folder);
    std::filesystem::create_directories(folder);
    save(folder / "rooms.lua", "return { count = 1 }\n");
    save(folder / "script.lua", REQUIRES_THE_ROOMS);

    const ScriptSystem &scripts = registry.addSystem<ScriptSystem, Script>(components, bus, Actions{}, folder);
    const Entity entity = registry.createEntity();

    registry.addComponent(entity, Script{"script.lua"});
    registry.updateSystems(0.5F);
    registry.updateSystems(0.5F);
    cr_assert_not(scripts.isRunning(entity));
}
