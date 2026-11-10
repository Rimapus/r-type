/*
** EPITECH PROJECT, 2026
** younity-engine/tests/scripting/LogLibrary
** File description:
** Unit tests of the log table of the scripts
*/

#include <criterion/criterion.h>

#include <filesystem>
#include <memory>
#include <string>
#include <vector>

#include "younity/core/EventBus.hpp"
#include "younity/ecs/ComponentRegistry.hpp"
#include "younity/ecs/Registry.hpp"
#include "younity/logger/Global.hpp"
#include "younity/logger/MemorySink.hpp"
#include "younity/logger/Record.hpp"
#include "younity/scripting/Actions.hpp"
#include "younity/scripting/components/Script.hpp"
#include "younity/scripting/systems/ScriptSystem.hpp"

using younity::EventBus;
using younity::ecs::ComponentRegistry;
using younity::ecs::Entity;
using younity::ecs::Registry;
using younity::logger::Level;
using younity::logger::MemorySink;
using younity::logger::Record;
using younity::scripting::Actions;
using younity::scripting::Script;
using younity::scripting::ScriptSystem;

namespace {

const std::filesystem::path FIXTURES = std::filesystem::path(YOUNITY_TESTS_DIR) / "scripting" / "fixtures";

} // namespace

Test(scripting_log_library, writes_the_message_with_its_position)
{
    auto sink = std::make_unique<MemorySink>();
    const MemorySink &records = *sink;
    const ComponentRegistry components;
    EventBus bus;
    Registry registry;
    const Entity entity = registry.createEntity();

    younity::logger::registry().addSink("scripts", std::move(sink));
    registry.addSystem<ScriptSystem, Script>(components, bus, Actions{}, FIXTURES);
    registry.addComponent(entity, Script{"says_hello.lua"});
    registry.updateSystems(0.5F);

    // Copied: removing the sink destroys it
    const std::vector<Record> written = records.getRecords();

    younity::logger::registry().removeSink("scripts");
    cr_assert_eq(written.size(), 1U);
    cr_assert_eq(written[0].getLevel(), Level::Warn);
    cr_assert(written[0].getCategory() == "scripting");
    cr_assert(written[0].getMessage().ends_with("says_hello.lua:5: hello"));
}

Test(scripting_log_library, message_must_be_a_string)
{
    const ComponentRegistry components;
    EventBus bus;
    Registry registry;
    const ScriptSystem &scripts = registry.addSystem<ScriptSystem, Script>(components, bus, Actions{}, FIXTURES);
    const Entity entity = registry.createEntity();

    registry.addComponent(entity, Script{"logs_a_table.lua"});
    registry.updateSystems(0.5F);
    cr_assert_not(scripts.isRunning(entity));
}
