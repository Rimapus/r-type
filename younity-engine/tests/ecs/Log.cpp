/*
** EPITECH PROJECT, 2026
** younity-engine/tests/ecs/Log
** File description:
** Generated ecs Log unit tests
*/

#include <criterion/criterion.h>

#include <memory>
#include <utility>

#include "younity/ecs/Log.hpp"
#include "younity/logger/Global.hpp"
#include "younity/logger/MemorySink.hpp"

namespace younity::ecs {

// Written like the code of the module: logger() is the one of the module, and logger:: is still the namespace
template <typename Component>
void logFromTheModule(logger::Level level)
{
    logger().log(level, "from a template");
}

} // namespace younity::ecs

Test(ecs_log, logger_is_named_after_the_module)
{
    cr_assert(younity::ecs::logger().getName() == "ecs");
}

Test(ecs_log, messages_of_the_module_have_its_name)
{
    auto sink = std::make_unique<younity::logger::MemorySink>();
    const younity::logger::MemorySink &memory = *sink;

    younity::logger::registry().addSink("memory", std::move(sink));
    younity::ecs::logFromTheModule<int>(younity::logger::Level::Warn);
    cr_assert_eq(memory.getRecords().size(), 1);
    cr_assert(memory.getRecords().front().getCategory() == "ecs");
}
