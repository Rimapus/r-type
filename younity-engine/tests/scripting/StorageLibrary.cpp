/*
** EPITECH PROJECT, 2026
** younity-engine/tests/scripting/StorageLibrary
** File description:
** StorageLibrary unit tests
*/

#include <criterion/criterion.h>

#include <filesystem>
#include <fstream>
#include <string>

#include "younity/core/EventBus.hpp"
#include "younity/ecs/ComponentRegistry.hpp"
#include "younity/ecs/Registry.hpp"
#include "younity/scripting/Host.hpp"
#include "younity/scripting/components/Script.hpp"
#include "younity/scripting/systems/ScriptSystem.hpp"

using younity::EventBus;
using younity::ecs::ComponentRegistry;
using younity::ecs::Entity;
using younity::ecs::Registry;
using younity::scripting::Host;
using younity::scripting::Script;
using younity::scripting::ScriptSystem;

namespace {

const std::filesystem::path FIXTURES = std::filesystem::path(YOUNITY_TESTS_DIR) / "scripting" / "fixtures";

// Criterion runs the tests in parallel: each test saves in its own folder, removed before it
std::filesystem::path savesFolder(const char *test)
{
    const std::filesystem::path folder = std::filesystem::temp_directory_path() / ("younity_saves_" + std::string(test));

    std::filesystem::remove_all(folder);
    return folder;
}

// Runs a script saving in a folder, then tells if it still runs
bool runs(const std::filesystem::path &saves, const std::string &fixture)
{
    const ComponentRegistry components;
    EventBus bus;
    Registry registry;
    Host host{};

    host.saves = saves;

    const ScriptSystem &scripts = registry.addSystem<ScriptSystem, Script>(components, bus, host, FIXTURES);
    const Entity entity = registry.createEntity();

    registry.addComponent(entity, Script{fixture});
    registry.updateSystems(0.5F);
    return scripts.isRunning(entity);
}

} // namespace

Test(scripting_storage_library, saves_a_table_and_loads_it_back)
{
    const std::filesystem::path saves = savesFolder("saved");

    cr_assert(runs(saves, "saves_and_loads.lua"));
    cr_assert(std::filesystem::exists(saves / "best.json"));
    cr_assert_not(std::filesystem::exists(saves / "best.json.tmp"));
}

Test(scripting_storage_library, missing_save_is_nil)
{
    cr_assert(runs(savesFolder("missing"), "loads_a_missing_save.lua"));
}

Test(scripting_storage_library, broken_save_is_nil)
{
    const std::filesystem::path saves = savesFolder("broken");

    std::filesystem::create_directories(saves);
    std::ofstream(saves / "broken.json") << "{ not json";
    cr_assert(runs(saves, "loads_a_broken_save.lua"));
}

Test(scripting_storage_library, name_with_a_path_stops_the_script)
{
    cr_assert_not(runs(savesFolder("named"), "saves_with_a_wrong_name.lua"));
}

Test(scripting_storage_library, function_stops_the_script)
{
    const std::filesystem::path saves = savesFolder("function");

    cr_assert_not(runs(saves, "saves_a_function.lua"));
    cr_assert_not(std::filesystem::exists(saves / "best.json"));
}

Test(scripting_storage_library, table_containing_itself_stops_the_script)
{
    cr_assert_not(runs(savesFolder("itself"), "saves_itself.lua"));
}

Test(scripting_storage_library, nothing_is_saved_without_a_folder)
{
    cr_assert(runs({}, "saves_without_a_folder.lua"));
}
