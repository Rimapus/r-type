/*
** EPITECH PROJECT, 2026
** younity-engine/tests/scripting/InputLibrary
** File description:
** Unit tests of the input table of the scripts
*/

#include <criterion/criterion.h>

#include <filesystem>
#include <set>
#include <string>
#include <vector>

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

// Runs the script reading the shoot action, then tells if it still runs
bool runsWith(const Actions &actions)
{
    const ComponentRegistry components;
    EventBus bus;
    Registry registry;
    const ScriptSystem &scripts = registry.addSystem<ScriptSystem, Script>(components, bus, actions, FIXTURES);
    const Entity entity = registry.createEntity();

    registry.addComponent(entity, Script{"checks_the_shoot_action.lua"});
    registry.updateSystems(0.5F);
    return scripts.isRunning(entity);
}

// Runs a script with the actions of the game, then tells if it still runs
bool runsScript(const Actions &actions, const std::string &fixture)
{
    const ComponentRegistry components;
    EventBus bus;
    Registry registry;
    const ScriptSystem &scripts = registry.addSystem<ScriptSystem, Script>(components, bus, actions, FIXTURES);
    const Entity entity = registry.createEntity();

    registry.addComponent(entity, Script{fixture});
    registry.updateSystems(0.5F);
    return scripts.isRunning(entity);
}

// Binds only the names of the keys and axes it knows
Actions bindingActions(std::vector<std::string> &calls)
{
    Actions actions;

    actions.bindKey = [&calls](const std::string &action, const std::string &key) {
        calls.push_back(action + " key " + key);
        return key == "Space";
    };
    actions.bindButton = [&calls](const std::string &action, unsigned int button) {
        calls.push_back(action + " button " + std::to_string(button));
    };
    actions.bindAxis = [&calls](const std::string &action, const std::string &axis, bool positive) {
        calls.push_back(action + " axis " + axis + (positive ? "+" : "-"));
        return axis == "Y";
    };
    actions.unbind = [&calls](const std::string &action) { calls.push_back(action + " unbound"); };
    actions.getBindings = [](const std::string &action) {
        return action == "jump" ? std::vector<std::string>{"Space"} : std::vector<std::string>{};
    };
    return actions;
}

} // namespace

Test(scripting_input_library, reads_a_pressed_action)
{
    const std::set<std::string> pressed{"shoot"};
    const auto isPressed = [&pressed](const std::string &action) { return pressed.contains(action); };

    cr_assert_not(runsWith(Actions{isPressed, isPressed, [](const std::string &) { return false; }}));
}

Test(scripting_input_library, reads_a_released_action)
{
    const auto never = [](const std::string &) { return false; };

    cr_assert(runsWith(Actions{never, never, never}));
}

Test(scripting_input_library, no_action_is_pressed_without_input)
{
    cr_assert(runsWith(Actions{}));
}

Test(scripting_input_library, binds_keys_buttons_and_axes)
{
    std::vector<std::string> calls;

    cr_assert(runsScript(bindingActions(calls), "binds_the_jump.lua"));
    cr_assert(calls == (std::vector<std::string>{"jump key Space", "jump button 0", "jump axis Y-", "dash unbound"}));
}

Test(scripting_input_library, unknown_key_stops_the_script)
{
    std::vector<std::string> calls;

    cr_assert_not(runsScript(bindingActions(calls), "binds_an_unknown_key.lua"));
}

Test(scripting_input_library, negative_button_stops_the_script)
{
    std::vector<std::string> calls;

    cr_assert_not(runsScript(bindingActions(calls), "binds_a_negative_button.lua"));
    cr_assert(calls.empty());
}

Test(scripting_input_library, nothing_is_bound_without_input)
{
    cr_assert(runsScript(Actions{}, "binds_without_input.lua"));
}
