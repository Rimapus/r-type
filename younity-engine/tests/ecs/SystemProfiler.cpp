/*
** EPITECH PROJECT, 2026
** younity-engine/tests/ecs/SystemProfiler
** File description:
** System name and profiler unit tests
*/

#include <criterion/criterion.h>

#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include "younity/ecs/Registry.hpp"

using younity::ecs::Entity;
using younity::ecs::Registry;
using younity::ecs::System;

namespace {

class MovementSystem : public System
{
public:
    void update(Registry &registry, const std::vector<Entity> &entities, float dt) override
    {
        (void)registry;
        (void)entities;
        (void)dt;
    }
};

} // namespace

Test(ecs_system_profiler, add_system_names_it_after_its_class)
{
    Registry registry;
    MovementSystem &system = registry.addSystem<MovementSystem>();

    cr_assert(system.getName() == "MovementSystem");
}

Test(ecs_system_profiler, set_name_replaces_the_default_name)
{
    Registry registry;
    MovementSystem &system = registry.addSystem<MovementSystem>();

    system.setName("movement");
    cr_assert(system.getName() == "movement");
}

Test(ecs_system_profiler, set_name_rejects_an_empty_name)
{
    Registry registry;
    MovementSystem &system = registry.addSystem<MovementSystem>();

    cr_assert_throw(system.setName(""), std::invalid_argument);
}

Test(ecs_system_profiler, profiles_each_system_once_per_update)
{
    Registry registry;
    std::vector<std::string> names;
    std::vector<double> durations;

    registry.addSystem<MovementSystem>();
    registry.addSystem<MovementSystem>();
    registry.setSystemProfiler([&names, &durations](std::string_view name, double seconds) {
        names.emplace_back(name);
        durations.push_back(seconds);
    });
    registry.updateSystems(0.01F);
    cr_assert_eq(names.size(), 2U);
    cr_assert(names[0] == "MovementSystem");
    cr_assert(names[1] == "MovementSystem");
    cr_assert_geq(durations[0], 0.0);
}

Test(ecs_system_profiler, profiles_the_name_set_on_the_system)
{
    Registry registry;
    std::string seen;
    MovementSystem &system = registry.addSystem<MovementSystem>();

    system.setName("movement");
    registry.setSystemProfiler([&seen](std::string_view name, double /* seconds */) {
        seen = name;
    });
    registry.updateSystems(0.0F);
    cr_assert(seen == "movement");
}

Test(ecs_system_profiler, an_empty_profiler_stops_the_calls)
{
    Registry registry;
    unsigned int calls = 0;

    registry.addSystem<MovementSystem>();
    registry.setSystemProfiler([&calls](std::string_view /* name */, double /* seconds */) {
        ++calls;
    });
    registry.updateSystems(0.0F);
    registry.setSystemProfiler({});
    registry.updateSystems(0.0F);
    cr_assert_eq(calls, 1U);
}
