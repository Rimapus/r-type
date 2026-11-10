/*
** EPITECH PROJECT, 2026
** younity-engine/tests/scripting/ScriptSystem
** File description:
** ScriptSystem unit tests
*/

#include <criterion/criterion.h>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <string>

#include "younity/core/EventBus.hpp"
#include "younity/ecs/ComponentRegistry.hpp"
#include "younity/ecs/Registry.hpp"
#include "younity/physics/CollisionEvent.hpp"
#include "younity/scripting/Actions.hpp"
#include "younity/scripting/components/Script.hpp"
#include "younity/scripting/systems/ScriptSystem.hpp"

using younity::EventBus;
using younity::ecs::ComponentRegistry;
using younity::ecs::Entity;
using younity::ecs::Registry;
using younity::physics::CollisionEvent;
using younity::scripting::Actions;
using younity::scripting::Script;
using younity::scripting::ScriptSystem;

namespace {

const std::filesystem::path FIXTURES = std::filesystem::path(YOUNITY_TESTS_DIR) / "scripting" / "fixtures";

// No component type: these scripts do not use the ecs table
const ComponentRegistry &components(void)
{
    static const ComponentRegistry registered;

    return registered;
}

// The system running the scripts of the fixtures folder
ScriptSystem &addScriptSystem(Registry &registry, EventBus &bus)
{
    return registry.addSystem<ScriptSystem, Script>(components(), bus, Actions{}, FIXTURES);
}

// An entity with the script of the given fixture
Entity createScripted(Registry &registry, const std::string &path)
{
    const Entity entity = registry.createEntity();

    registry.addComponent(entity, Script{path});
    return entity;
}

// Criterion runs the tests in parallel: each test uses its own folder, removed before it
std::filesystem::path testFolder(const char *test)
{
    const std::filesystem::path folder = std::filesystem::temp_directory_path() / ("younity_scripts_" + std::string(test));

    std::filesystem::remove_all(folder);
    std::filesystem::create_directories(folder);
    return folder;
}

// Save a script, written after the previous save even on a file system that keeps the time in whole seconds
void saveScript(const std::filesystem::path &file, const std::string &code)
{
    const bool existed = std::filesystem::exists(file);
    const std::filesystem::file_time_type previous = existed ? std::filesystem::last_write_time(file) : std::filesystem::file_time_type{};

    std::ofstream(file) << code;
    if (existed) {
        std::filesystem::last_write_time(file, previous + std::chrono::seconds(2));
    }
}

const char *const UPDATES_ONCE = "local Script = { updates = 0 }\n"
                                 "function Script.update(entity, dt)\n"
                                 "    Script.updates = Script.updates + 1\n"
                                 "    if Script.updates > 1 then error('updated twice') end\n"
                                 "end\n"
                                 "return Script\n";

} // namespace

Test(scripting_script_system, does_not_run_before_the_first_update)
{
    Registry registry;
    EventBus bus;
    const ScriptSystem &scripts = addScriptSystem(registry, bus);
    const Entity entity = createScripted(registry, "empty.lua");

    cr_assert_not(scripts.isRunning(entity));
}

Test(scripting_script_system, runs_a_script_without_functions)
{
    Registry registry;
    EventBus bus;
    const ScriptSystem &scripts = addScriptSystem(registry, bus);
    const Entity entity = createScripted(registry, "empty.lua");

    registry.updateSystems(0.5F);
    cr_assert(scripts.isRunning(entity));
}

Test(scripting_script_system, update_gets_the_entity_and_dt)
{
    Registry registry;
    EventBus bus;
    const ScriptSystem &scripts = addScriptSystem(registry, bus);
    const Entity entity = createScripted(registry, "checks_its_arguments.lua");

    registry.updateSystems(0.5F);
    cr_assert_not(scripts.isRunning(entity));
}

Test(scripting_script_system, start_runs_once_before_the_updates)
{
    Registry registry;
    EventBus bus;
    const ScriptSystem &scripts = addScriptSystem(registry, bus);
    const Entity entity = createScripted(registry, "starts_first.lua");

    registry.updateSystems(0.5F);
    registry.updateSystems(0.5F);
    cr_assert(scripts.isRunning(entity));
}

Test(scripting_script_system, update_runs_at_each_step)
{
    Registry registry;
    EventBus bus;
    const ScriptSystem &scripts = addScriptSystem(registry, bus);
    const Entity entity = createScripted(registry, "updates_once.lua");

    registry.updateSystems(0.5F);
    cr_assert(scripts.isRunning(entity));
    registry.updateSystems(0.5F);
    cr_assert_not(scripts.isRunning(entity));
}

Test(scripting_script_system, each_entity_has_its_own_state)
{
    Registry registry;
    EventBus bus;
    const ScriptSystem &scripts = addScriptSystem(registry, bus);
    const Entity first = createScripted(registry, "updates_once.lua");
    const Entity second = createScripted(registry, "updates_once.lua");

    registry.updateSystems(0.5F);
    cr_assert(scripts.isRunning(first));
    cr_assert(scripts.isRunning(second));
}

Test(scripting_script_system, error_stops_only_the_failing_script)
{
    Registry registry;
    EventBus bus;
    const ScriptSystem &scripts = addScriptSystem(registry, bus);
    const Entity failing = createScripted(registry, "updates_once.lua");
    const Entity working = createScripted(registry, "empty.lua");

    registry.updateSystems(0.5F);
    registry.updateSystems(0.5F);
    registry.updateSystems(0.5F);
    cr_assert_not(scripts.isRunning(failing));
    cr_assert(scripts.isRunning(working));
}

Test(scripting_script_system, missing_file_does_not_run)
{
    Registry registry;
    EventBus bus;
    const ScriptSystem &scripts = addScriptSystem(registry, bus);
    const Entity entity = createScripted(registry, "does_not_exist.lua");

    registry.updateSystems(0.5F);
    cr_assert_not(scripts.isRunning(entity));
}

Test(scripting_script_system, syntax_error_does_not_run)
{
    Registry registry;
    EventBus bus;
    const ScriptSystem &scripts = addScriptSystem(registry, bus);
    const Entity entity = createScripted(registry, "syntax_error.lua");

    registry.updateSystems(0.5F);
    cr_assert_not(scripts.isRunning(entity));
}

Test(scripting_script_system, script_must_return_a_table)
{
    Registry registry;
    EventBus bus;
    const ScriptSystem &scripts = addScriptSystem(registry, bus);
    const Entity entity = createScripted(registry, "returns_a_number.lua");

    registry.updateSystems(0.5F);
    cr_assert_not(scripts.isRunning(entity));
}

Test(scripting_script_system, update_must_be_a_function)
{
    Registry registry;
    EventBus bus;
    const ScriptSystem &scripts = addScriptSystem(registry, bus);
    const Entity entity = createScripted(registry, "update_is_a_number.lua");

    registry.updateSystems(0.5F);
    cr_assert_not(scripts.isRunning(entity));
}

Test(scripting_script_system, scripts_have_no_io_library)
{
    Registry registry;
    EventBus bus;
    const ScriptSystem &scripts = addScriptSystem(registry, bus);
    const Entity entity = createScripted(registry, "opens_a_file.lua");

    registry.updateSystems(0.5F);
    cr_assert_not(scripts.isRunning(entity));
}

Test(scripting_script_system, scripts_cannot_run_a_file)
{
    Registry registry;
    EventBus bus;
    const ScriptSystem &scripts = addScriptSystem(registry, bus);
    const Entity entity = createScripted(registry, "runs_a_file.lua");

    registry.updateSystems(0.5F);
    cr_assert_not(scripts.isRunning(entity));
}

Test(scripting_script_system, functions_are_looked_up_without_the_metatable)
{
    Registry registry;
    EventBus bus;
    const ScriptSystem &scripts = addScriptSystem(registry, bus);
    const Entity entity = createScripted(registry, "fails_to_index.lua");

    registry.updateSystems(0.5F);
    cr_assert(scripts.isRunning(entity));
}

Test(scripting_script_system, new_path_starts_the_script_again)
{
    Registry registry;
    EventBus bus;
    const ScriptSystem &scripts = addScriptSystem(registry, bus);
    const Entity entity = createScripted(registry, "updates_once.lua");

    registry.updateSystems(0.5F);
    registry.updateSystems(0.5F);
    registry.getComponent<Script>(entity).path = "empty.lua";
    registry.updateSystems(0.5F);
    cr_assert(scripts.isRunning(entity));
}

Test(scripting_script_system, removed_script_stops)
{
    Registry registry;
    EventBus bus;
    const ScriptSystem &scripts = addScriptSystem(registry, bus);
    const Entity entity = createScripted(registry, "empty.lua");

    registry.updateSystems(0.5F);
    registry.removeComponent<Script>(entity);
    registry.updateSystems(0.5F);
    cr_assert_not(scripts.isRunning(entity));
}

Test(scripting_script_system, new_entity_with_a_reused_id_starts_its_own_script)
{
    Registry registry;
    EventBus bus;
    const ScriptSystem &scripts = addScriptSystem(registry, bus);
    const Entity destroyed = createScripted(registry, "updates_once.lua");

    registry.updateSystems(0.5F);
    registry.destroyEntity(destroyed);

    const Entity created = createScripted(registry, "updates_once.lua");

    cr_assert_eq(created, destroyed);
    registry.updateSystems(0.5F);
    cr_assert(scripts.isRunning(created));
}

Test(scripting_script_system, on_collision_gets_the_entity_and_the_other_one)
{
    Registry registry;
    EventBus bus;
    const ScriptSystem &scripts = addScriptSystem(registry, bus);
    const Entity entity = createScripted(registry, "fails_on_collision.lua");
    const Entity other = registry.createEntity();

    registry.updateSystems(0.5F);
    // The entity is the second of the pair: the script still gets it first
    bus.publish(CollisionEvent{other, entity});
    registry.updateSystems(0.5F);
    cr_assert_not(scripts.isRunning(entity));
}

Test(scripting_script_system, on_collision_is_not_called_without_a_collision)
{
    Registry registry;
    EventBus bus;
    const ScriptSystem &scripts = addScriptSystem(registry, bus);
    const Entity entity = createScripted(registry, "fails_on_collision.lua");
    const Entity first = registry.createEntity();
    const Entity second = registry.createEntity();

    registry.updateSystems(0.5F);
    bus.publish(CollisionEvent{first, second});
    registry.updateSystems(0.5F);
    cr_assert(scripts.isRunning(entity));
}

Test(scripting_script_system, collision_is_given_once)
{
    Registry registry;
    EventBus bus;
    const ScriptSystem &scripts = addScriptSystem(registry, bus);
    const Entity entity = createScripted(registry, "collides_once.lua");
    const Entity other = registry.createEntity();

    registry.updateSystems(0.5F);
    bus.publish(CollisionEvent{entity, other});
    registry.updateSystems(0.5F);
    registry.updateSystems(0.5F);
    cr_assert(scripts.isRunning(entity));
}

Test(scripting_script_system, on_collision_must_be_a_function)
{
    Registry registry;
    EventBus bus;
    const ScriptSystem &scripts = addScriptSystem(registry, bus);
    const Entity entity = createScripted(registry, "on_collision_is_a_number.lua");

    registry.updateSystems(0.5F);
    cr_assert_not(scripts.isRunning(entity));
}

Test(scripting_script_system, collision_before_the_start_is_not_given)
{
    Registry registry;
    EventBus bus;
    const ScriptSystem &scripts = addScriptSystem(registry, bus);
    const Entity entity = createScripted(registry, "fails_on_collision.lua");
    const Entity other = registry.createEntity();

    bus.publish(CollisionEvent{entity, other});
    registry.updateSystems(0.5F);
    cr_assert(scripts.isRunning(entity));
}

Test(scripting_script_system, collision_of_a_destroyed_entity_is_not_given_to_its_id)
{
    Registry registry;
    EventBus bus;
    const ScriptSystem &scripts = addScriptSystem(registry, bus);
    const Entity destroyed = createScripted(registry, "fails_on_collision.lua");
    const Entity other = registry.createEntity();

    registry.updateSystems(0.5F);
    bus.publish(CollisionEvent{destroyed, other});
    registry.destroyEntity(destroyed);

    const Entity created = createScripted(registry, "fails_on_collision.lua");

    cr_assert_eq(created, destroyed);
    registry.updateSystems(0.5F);
    cr_assert(scripts.isRunning(created));
}

Test(scripting_script_system, destroyed_entity_stops_its_script_in_the_step)
{
    Registry registry;
    EventBus bus;
    const ScriptSystem &scripts = addScriptSystem(registry, bus);
    const Entity entity = createScripted(registry, "destroys_on_collision.lua");
    const Entity first = registry.createEntity();
    const Entity second = registry.createEntity();

    registry.updateSystems(0.5F);
    bus.publish(CollisionEvent{entity, first});
    bus.publish(CollisionEvent{second, entity});
    registry.updateSystems(0.5F);
    cr_assert(scripts.isRunning(entity));
    cr_assert_not(registry.isAlive(entity));
}

Test(scripting_script_system, saved_script_runs_again_once_fixed)
{
    const std::filesystem::path folder = testFolder("fixed");
    Registry registry;
    EventBus bus;
    const ScriptSystem &scripts = registry.addSystem<ScriptSystem, Script>(components(), bus, Actions{}, folder);
    const Entity entity = createScripted(registry, "script.lua");

    saveScript(folder / "script.lua", "return 1\n");
    registry.updateSystems(0.5F);
    cr_assert_not(scripts.isRunning(entity));
    saveScript(folder / "script.lua", "return {}\n");
    registry.updateSystems(0.5F);
    cr_assert(scripts.isRunning(entity));
}

Test(scripting_script_system, saved_script_starts_again_with_a_new_table)
{
    const std::filesystem::path folder = testFolder("restarted");
    Registry registry;
    EventBus bus;
    const ScriptSystem &scripts = registry.addSystem<ScriptSystem, Script>(components(), bus, Actions{}, folder);
    const Entity entity = createScripted(registry, "script.lua");

    saveScript(folder / "script.lua", UPDATES_ONCE);
    registry.updateSystems(0.5F);
    saveScript(folder / "script.lua", UPDATES_ONCE);
    registry.updateSystems(0.5F);
    cr_assert(scripts.isRunning(entity));
}

Test(scripting_script_system, missing_script_runs_once_created)
{
    const std::filesystem::path folder = testFolder("created");
    Registry registry;
    EventBus bus;
    const ScriptSystem &scripts = registry.addSystem<ScriptSystem, Script>(components(), bus, Actions{}, folder);
    const Entity entity = createScripted(registry, "script.lua");

    registry.updateSystems(0.5F);
    cr_assert_not(scripts.isRunning(entity));
    saveScript(folder / "script.lua", "return {}\n");
    registry.updateSystems(0.5F);
    cr_assert(scripts.isRunning(entity));
}

Test(scripting_script_system, deleted_script_keeps_running)
{
    const std::filesystem::path folder = testFolder("deleted");
    Registry registry;
    EventBus bus;
    const ScriptSystem &scripts = registry.addSystem<ScriptSystem, Script>(components(), bus, Actions{}, folder);
    const Entity entity = createScripted(registry, "script.lua");

    saveScript(folder / "script.lua", "return {}\n");
    registry.updateSystems(0.5F);
    // An editor may delete the file for a moment while it saves it
    std::filesystem::remove(folder / "script.lua");
    registry.updateSystems(0.5F);
    cr_assert(scripts.isRunning(entity));
}

Test(scripting_script_system, unsaved_script_keeps_its_table)
{
    const std::filesystem::path folder = testFolder("kept");
    Registry registry;
    EventBus bus;
    const ScriptSystem &scripts = registry.addSystem<ScriptSystem, Script>(components(), bus, Actions{}, folder);
    const Entity entity = createScripted(registry, "script.lua");

    saveScript(folder / "script.lua", UPDATES_ONCE);
    registry.updateSystems(0.5F);
    registry.updateSystems(0.5F);
    cr_assert_not(scripts.isRunning(entity));
}
