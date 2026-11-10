/*
** EPITECH PROJECT, 2026
** younity-engine/tests/ecs/ComponentInfo
** File description:
** ComponentInfo unit tests
*/

#include <criterion/criterion.h>

#include <cstdint>
#include <optional>
#include <stdexcept>
#include <string>
#include <variant>
#include <vector>

#include "younity/ecs/ComponentInfo.hpp"

using younity::Color;
using younity::Recti;
using younity::Vector2f;
using younity::ecs::ComponentInfo;
using younity::ecs::Entity;
using younity::ecs::FieldType;
using younity::ecs::FieldValue;
using younity::ecs::Registry;

namespace {

struct Player
{
    std::string name;
    Vector2f position{};
    std::uint8_t slot = 0;
};

struct Health
{
    int value = 100;
};

struct Everything
{
    bool flag = false;
    int integer = 0;
    float decimal = 0.0F;
    std::string text;
    Vector2f point{};
    Color color{};
    std::optional<Recti> area;
    std::vector<Recti> frames;
};

struct Clip
{
    std::vector<Recti> frames;
    float frameDuration = 0.0F;
};

struct Animated
{
    Clip clip;
};

ComponentInfo makePlayerInfo(void)
{
    ComponentInfo info = ComponentInfo::create<Player>("Player");

    info.field("name", &Player::name).field("position", &Player::position).field("slot", &Player::slot);
    return info;
}

} // namespace

Test(ecs_component_info, name_and_fields_in_their_order)
{
    const ComponentInfo info = makePlayerInfo();

    cr_assert_eq(info.getName(), "Player");
    cr_assert(info.getFieldNames() == (std::vector<std::string>{"name", "position", "slot"}));
}

Test(ecs_component_info, field_types)
{
    ComponentInfo info = ComponentInfo::create<Everything>("Everything");

    info.field("flag", &Everything::flag)
        .field("integer", &Everything::integer)
        .field("decimal", &Everything::decimal)
        .field("text", &Everything::text)
        .field("point", &Everything::point)
        .field("color", &Everything::color)
        .field("area", &Everything::area)
        .field("frames", &Everything::frames);
    cr_assert_eq(info.getFieldType("flag"), FieldType::Bool);
    cr_assert_eq(info.getFieldType("integer"), FieldType::Int);
    cr_assert_eq(info.getFieldType("decimal"), FieldType::Float);
    cr_assert_eq(info.getFieldType("text"), FieldType::String);
    cr_assert_eq(info.getFieldType("point"), FieldType::Vector2f);
    cr_assert_eq(info.getFieldType("color"), FieldType::Color);
    cr_assert_eq(info.getFieldType("area"), FieldType::OptionalRecti);
    cr_assert_eq(info.getFieldType("frames"), FieldType::RectiVector);
}

Test(ecs_component_info, nested_field_is_read_and_written)
{
    ComponentInfo info = ComponentInfo::create<Animated>("Animated");
    Registry registry;
    const Entity entity = registry.createEntity();

    info.field("frames", &Animated::clip, &Clip::frames)
        .field("frameDuration", &Animated::clip, &Clip::frameDuration);
    info.add(registry, entity);
    info.setField(registry, entity, "frames", std::vector<Recti>{
                                                  Recti{0, 0, 8, 8},
                                                  Recti{8, 0, 8, 8}
    });
    info.setField(registry, entity, "frameDuration", 0.25F);
    cr_assert(registry.getComponent<Animated>(entity).clip.frames ==
              (std::vector<Recti>{
                  Recti{0, 0, 8, 8},
                  Recti{8, 0, 8, 8}
    }));
    cr_assert_float_eq(registry.getComponent<Animated>(entity).clip.frameDuration, 0.25F, 0.0001F);
    cr_assert_eq(info.getField(registry, entity, "frameDuration"), FieldValue(0.25F));
}

Test(ecs_component_info, field_is_read_and_written_through_functions)
{
    ComponentInfo info = ComponentInfo::create<Health>("Health");
    Registry registry;
    const Entity entity = registry.createEntity();

    // The value is shown as a percentage of 200 points
    info.field<Health, int>(
        "percent", [](const Health &health) { return health.value / 2; },
        [](Health &health, int percent) {
            if (percent < 0) {
                throw std::invalid_argument("negative percentage");
            }
            health.value = percent * 2;
        });
    info.add(registry, entity);
    cr_assert_eq(info.getField(registry, entity, "percent"), FieldValue(50));
    info.setField(registry, entity, "percent", 25);
    cr_assert_eq(registry.getComponent<Health>(entity).value, 50);
    cr_assert_throw(info.setField(registry, entity, "percent", -1), std::invalid_argument);
    cr_assert_eq(registry.getComponent<Health>(entity).value, 50);
}

Test(ecs_component_info, smaller_integer_field_is_an_int)
{
    const ComponentInfo info = makePlayerInfo();

    cr_assert_eq(info.getFieldType("slot"), FieldType::Int);
}

Test(ecs_component_info, unknown_field_type_throws)
{
    const ComponentInfo info = makePlayerInfo();

    cr_assert_throw((void)info.getFieldType("health"), std::out_of_range);
}

Test(ecs_component_info, add_has_and_remove_by_name)
{
    const ComponentInfo info = ComponentInfo::create<Health>("Health");
    Registry registry;
    const Entity entity = registry.createEntity();

    info.add(registry, entity);
    cr_assert(info.has(registry, entity));
    cr_assert_eq(registry.getComponent<Health>(entity).value, 100);
    info.remove(registry, entity);
    cr_assert_not(info.has(registry, entity));
}

Test(ecs_component_info, set_then_get_a_field)
{
    const ComponentInfo info = makePlayerInfo();
    Registry registry;
    const Entity entity = registry.createEntity();

    info.add(registry, entity);
    info.setField(registry, entity, "position", Vector2f{3.0F, 4.0F});
    cr_assert(registry.getComponent<Player>(entity).position == (Vector2f{3.0F, 4.0F}));
    cr_assert(info.getField(registry, entity, "position") == FieldValue(Vector2f{3.0F, 4.0F}));
}

Test(ecs_component_info, integer_field_is_an_int)
{
    const ComponentInfo info = makePlayerInfo();
    Registry registry;
    const Entity entity = registry.createEntity();

    info.add(registry, entity);
    info.setField(registry, entity, "slot", 3);
    cr_assert_eq(registry.getComponent<Player>(entity).slot, 3);
    cr_assert_eq(std::get<int>(info.getField(registry, entity, "slot")), 3);
}

Test(ecs_component_info, integer_out_of_range_throws)
{
    const ComponentInfo info = makePlayerInfo();
    Registry registry;
    const Entity entity = registry.createEntity();

    info.add(registry, entity);
    cr_assert_throw(info.setField(registry, entity, "slot", 256), std::out_of_range);
    cr_assert_throw(info.setField(registry, entity, "slot", -1), std::out_of_range);
}

Test(ecs_component_info, value_of_another_type_throws)
{
    const ComponentInfo info = makePlayerInfo();
    Registry registry;
    const Entity entity = registry.createEntity();

    info.add(registry, entity);
    cr_assert_throw(info.setField(registry, entity, "name", 1.0F), std::invalid_argument);
}

Test(ecs_component_info, unknown_field_throws)
{
    const ComponentInfo info = makePlayerInfo();
    Registry registry;
    const Entity entity = registry.createEntity();

    info.add(registry, entity);
    cr_assert_throw((void)info.getField(registry, entity, "health"), std::out_of_range);
}

Test(ecs_component_info, missing_component_throws)
{
    const ComponentInfo info = makePlayerInfo();
    Registry registry;
    const Entity entity = registry.createEntity();

    cr_assert_throw((void)info.getField(registry, entity, "name"), std::out_of_range);
}

Test(ecs_component_info, field_used_twice_throws)
{
    ComponentInfo info = makePlayerInfo();

    cr_assert_throw(info.field("name", &Player::name), std::invalid_argument);
}

Test(ecs_component_info, field_of_another_component_throws)
{
    ComponentInfo info = ComponentInfo::create<Player>("Player");

    cr_assert_throw(info.field("value", &Health::value), std::invalid_argument);
}
