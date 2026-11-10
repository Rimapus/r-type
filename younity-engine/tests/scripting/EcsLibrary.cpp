/*
** EPITECH PROJECT, 2026
** younity-engine/tests/scripting/EcsLibrary
** File description:
** Unit tests of the ecs table of the scripts
*/

#include <criterion/criterion.h>

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

#include "younity/core/Color.hpp"
#include "younity/core/EventBus.hpp"
#include "younity/core/Rect.hpp"
#include "younity/ecs/ComponentRegistry.hpp"
#include "younity/ecs/Components.hpp"
#include "younity/ecs/Registry.hpp"
#include "younity/ecs/components/Transform.hpp"
#include "younity/ecs/components/Velocity.hpp"
#include "younity/scripting/Actions.hpp"
#include "younity/scripting/Components.hpp"
#include "younity/scripting/components/Script.hpp"
#include "younity/scripting/systems/ScriptSystem.hpp"

using younity::Color;
using younity::EventBus;
using younity::Recti;
using younity::ecs::ComponentRegistry;
using younity::ecs::Entity;
using younity::ecs::Registry;
using younity::ecs::Transform;
using younity::ecs::Velocity;
using younity::scripting::Actions;
using younity::scripting::Script;
using younity::scripting::ScriptSystem;

namespace {

const std::filesystem::path FIXTURES = std::filesystem::path(YOUNITY_TESTS_DIR) / "scripting" / "fixtures";

// A component with a field of each type a script reads and writes
struct Sample
{
    bool on = false;
    int count = 0;
    float speed = 0.0F;
    std::string name = "sample";
    Color color{};
    std::optional<Recti> area;
    std::vector<Recti> frames;
};

// The components of the ecs and scripting modules, and Sample
const ComponentRegistry &components(void)
{
    static const ComponentRegistry registered = [] {
        ComponentRegistry added;

        younity::ecs::registerComponents(added);
        younity::scripting::registerComponents(added);
        added.add<Sample>("Sample")
            .field("on", &Sample::on)
            .field("count", &Sample::count)
            .field("speed", &Sample::speed)
            .field("name", &Sample::name)
            .field("color", &Sample::color)
            .field("area", &Sample::area)
            .field("frames", &Sample::frames);
        return added;
    }();

    return registered;
}

// The system running the scripts of the fixtures folder
ScriptSystem &addScriptSystem(Registry &registry)
{
    static EventBus bus;

    return registry.addSystem<ScriptSystem, Script>(components(), bus, Actions{}, FIXTURES);
}

// An entity with a Transform and the script of the given fixture
Entity createScripted(Registry &registry, const std::string &path)
{
    const Entity entity = registry.createEntity();

    registry.addComponent(entity, Transform{});
    registry.addComponent(entity, Script{path});
    return entity;
}

} // namespace

Test(scripting_ecs_library, gets_and_sets_a_component)
{
    Registry registry;
    const ScriptSystem &scripts = addScriptSystem(registry);
    const Entity entity = createScripted(registry, "moves_its_entity.lua");

    registry.updateSystems(0.5F);
    registry.updateSystems(0.5F);
    cr_assert(scripts.isRunning(entity));
    cr_assert_float_eq(registry.getComponent<Transform>(entity).position.x, 2.0F, 1e-6F);
    cr_assert_float_eq(registry.getComponent<Transform>(entity).scale.x, 1.0F, 1e-6F);
}

Test(scripting_ecs_library, creates_an_entity)
{
    Registry registry;
    const ScriptSystem &scripts = addScriptSystem(registry);
    const Entity entity = createScripted(registry, "creates_an_entity.lua");

    registry.updateSystems(0.5F);
    cr_assert(scripts.isRunning(entity));

    const auto created = registry.getEntitiesWith<Velocity>();

    cr_assert_eq(created.size(), 1U);
    cr_assert_float_eq(registry.getComponent<Velocity>(created[0]).value.y, 3.0F, 1e-6F);
}

Test(scripting_ecs_library, destroys_an_entity_at_the_end_of_the_step)
{
    Registry registry;

    addScriptSystem(registry);
    const Entity entity = createScripted(registry, "destroys_itself.lua");

    registry.updateSystems(0.5F);
    cr_assert_not(registry.isAlive(entity));
}

Test(scripting_ecs_library, adds_and_removes_components)
{
    Registry registry;
    const ScriptSystem &scripts = addScriptSystem(registry);
    const Entity entity = createScripted(registry, "adds_and_removes.lua");

    registry.updateSystems(0.5F);
    cr_assert(scripts.isRunning(entity));
    cr_assert(registry.hasComponent<Velocity>(entity));
    cr_assert_not(registry.hasComponent<Transform>(entity));
}

Test(scripting_ecs_library, writes_every_field_type)
{
    Registry registry;
    const ScriptSystem &scripts = addScriptSystem(registry);
    const Entity entity = createScripted(registry, "writes_every_field_type.lua");

    registry.addComponent(entity, Sample{});
    registry.updateSystems(0.5F);
    cr_assert(scripts.isRunning(entity));

    const Sample &sample = registry.getComponent<Sample>(entity);

    cr_assert(sample.on);
    cr_assert_eq(sample.count, 4);
    cr_assert_float_eq(sample.speed, 1.5F, 1e-6F);
    cr_assert_eq(sample.name, "sample!");
    cr_assert(sample.color == (Color{1, 2, 3, 255}));
    cr_assert(sample.area == (Recti{1, 2, 3, 4}));
    cr_assert(sample.frames == (std::vector<Recti>{
                                   Recti{0, 0, 8, 8},
                                   Recti{8, 0, 8, 8}
    }));
}

Test(scripting_ecs_library, missing_component_stops_the_script)
{
    Registry registry;
    const ScriptSystem &scripts = addScriptSystem(registry);
    const Entity entity = createScripted(registry, "gets_a_missing_component.lua");

    registry.updateSystems(0.5F);
    cr_assert_not(scripts.isRunning(entity));
}

Test(scripting_ecs_library, unknown_component_stops_the_script)
{
    Registry registry;
    const ScriptSystem &scripts = addScriptSystem(registry);
    const Entity entity = createScripted(registry, "gets_an_unknown_component.lua");

    registry.updateSystems(0.5F);
    cr_assert_not(scripts.isRunning(entity));
}

Test(scripting_ecs_library, dead_entity_stops_the_script)
{
    Registry registry;
    const ScriptSystem &scripts = addScriptSystem(registry);
    const Entity entity = createScripted(registry, "gets_a_dead_entity.lua");

    registry.updateSystems(0.5F);
    cr_assert_not(scripts.isRunning(entity));
}

Test(scripting_ecs_library, wrong_field_leaves_the_component_unchanged)
{
    Registry registry;
    const ScriptSystem &scripts = addScriptSystem(registry);
    const Entity entity = createScripted(registry, "sets_a_wrong_field.lua");

    registry.updateSystems(0.5F);
    cr_assert_not(scripts.isRunning(entity));
    cr_assert_float_eq(registry.getComponent<Transform>(entity).rotation, 0.0F, 1e-6F);
}

Test(scripting_ecs_library, unknown_field_stops_the_script)
{
    Registry registry;
    const ScriptSystem &scripts = addScriptSystem(registry);
    const Entity entity = createScripted(registry, "sets_an_unknown_field.lua");

    registry.updateSystems(0.5F);
    cr_assert_not(scripts.isRunning(entity));
}

Test(scripting_ecs_library, errors_give_the_line_of_the_call)
{
    Registry registry;
    const ScriptSystem &scripts = addScriptSystem(registry);
    const Entity entity = createScripted(registry, "locates_its_errors.lua");

    registry.updateSystems(0.5F);
    cr_assert(scripts.isRunning(entity));
}

Test(scripting_ecs_library, reads_back_the_fields_it_writes)
{
    Registry registry;
    const ScriptSystem &scripts = addScriptSystem(registry);
    const Entity entity = createScripted(registry, "reads_back_its_fields.lua");

    registry.addComponent(entity, Sample{});
    registry.updateSystems(0.5F);
    cr_assert(scripts.isRunning(entity));
}

Test(scripting_ecs_library, wrong_ids_are_not_alive)
{
    Registry registry;
    const ScriptSystem &scripts = addScriptSystem(registry);
    const Entity entity = createScripted(registry, "checks_wrong_ids.lua");

    registry.updateSystems(0.5F);
    cr_assert(scripts.isRunning(entity));
}

Test(scripting_ecs_library, infinite_number_stops_the_script)
{
    Registry registry;
    const ScriptSystem &scripts = addScriptSystem(registry);
    const Entity entity = createScripted(registry, "sets_an_infinite_position.lua");

    registry.updateSystems(0.5F);
    cr_assert_not(scripts.isRunning(entity));
    cr_assert_float_eq(registry.getComponent<Transform>(entity).position.x, 0.0F, 1e-6F);
}

Test(scripting_ecs_library, color_channel_out_of_range_stops_the_script)
{
    Registry registry;
    const ScriptSystem &scripts = addScriptSystem(registry);
    const Entity entity = createScripted(registry, "sets_a_wrong_color.lua");

    registry.addComponent(entity, Sample{});
    registry.updateSystems(0.5F);
    cr_assert_not(scripts.isRunning(entity));
    cr_assert(registry.getComponent<Sample>(entity).color == Color{});
}

Test(scripting_ecs_library, wrong_frame_stops_the_script)
{
    Registry registry;
    const ScriptSystem &scripts = addScriptSystem(registry);
    const Entity entity = createScripted(registry, "sets_a_wrong_frame.lua");

    registry.addComponent(entity, Sample{});
    registry.updateSystems(0.5F);
    cr_assert_not(scripts.isRunning(entity));
    cr_assert(registry.getComponent<Sample>(entity).frames.empty());
}

Test(scripting_ecs_library, script_removed_earlier_in_the_step_does_not_run)
{
    Registry registry;
    const ScriptSystem &scripts = addScriptSystem(registry);
    const Entity remover = createScripted(registry, "removes_the_next_script.lua");
    const Entity removed = createScripted(registry, "empty.lua");

    cr_assert_eq(removed.getId(), remover.getId() + 1);
    registry.updateSystems(0.5F);
    cr_assert(scripts.isRunning(remover));
    cr_assert_not(scripts.isRunning(removed));
}
