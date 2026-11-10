/*
** EPITECH PROJECT, 2026
** younity-engine/tests/scripting/GameLibrary
** File description:
** GameLibrary unit tests
*/

#include <criterion/criterion.h>

#include <filesystem>
#include <format>
#include <string>
#include <vector>

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

// Runs a script with what the game gives, then tells if it still runs
bool runs(const Host &host, const std::string &fixture)
{
    const ComponentRegistry components;
    EventBus bus;
    Registry registry;
    const ScriptSystem &scripts = registry.addSystem<ScriptSystem, Script>(components, bus, host, FIXTURES);
    const Entity entity = registry.createEntity();

    registry.addComponent(entity, Script{fixture});
    registry.updateSystems(0.5F);
    return scripts.isRunning(entity);
}

// Writes each call of the scripts to the game loop
Host recordingHost(std::vector<std::string> &calls)
{
    Host host{};

    host.game.loadScene = [&calls](const std::string &path) { calls.push_back("scene " + path); };
    host.game.quit = [&calls] { calls.emplace_back("quit"); };
    host.game.setTimeScale = [&calls](float scale) { calls.push_back(std::format("time scale {}", scale)); };
    return host;
}

} // namespace

Test(scripting_game_library, asks_the_game_loop)
{
    std::vector<std::string> calls;

    cr_assert(runs(recordingHost(calls), "drives_the_game.lua"));
    cr_assert(calls == (std::vector<std::string>{"scene scenes/title.json", "time scale 0.5", "quit"}));
}

Test(scripting_game_library, negative_time_scale_stops_the_script)
{
    std::vector<std::string> calls;

    cr_assert_not(runs(recordingHost(calls), "sets_a_negative_time_scale.lua"));
    cr_assert(calls.empty());
}

Test(scripting_game_library, scene_outside_the_assets_stops_the_script)
{
    std::vector<std::string> calls;

    cr_assert_not(runs(recordingHost(calls), "loads_a_scene_outside_the_assets.lua"));
    cr_assert(calls.empty());
}

Test(scripting_game_library, nothing_happens_without_a_game_loop)
{
    cr_assert(runs(Host{}, "drives_the_game.lua"));
}
