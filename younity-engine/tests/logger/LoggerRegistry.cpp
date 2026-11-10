/*
** EPITECH PROJECT, 2026
** younity-engine/tests/logger/LoggerRegistry
** File description:
** LoggerRegistry unit tests
*/

#include <criterion/criterion.h>

#include <memory>
#include <thread>
#include <utility>
#include <vector>

#include "younity/logger/Level.hpp"
#include "younity/logger/Logger.hpp"
#include "younity/logger/LoggerRegistry.hpp"
#include "younity/logger/MemorySink.hpp"

using younity::logger::Level;
using younity::logger::Logger;
using younity::logger::LoggerRegistry;
using younity::logger::MemorySink;

namespace {

// Adds a memory sink to the registry, and keeps access to it to read the records
MemorySink &addMemorySink(LoggerRegistry &registry)
{
    auto sink = std::make_unique<MemorySink>();
    MemorySink &memory = *sink;

    registry.addSink("memory", std::move(sink));
    return memory;
}

} // namespace

Test(logger_logger_registry, get_creates_a_logger_with_the_name)
{
    LoggerRegistry registry;

    cr_assert(registry.get("ecs").getName() == "ecs");
}

Test(logger_logger_registry, get_returns_the_same_logger_for_a_name)
{
    LoggerRegistry registry;

    cr_assert_eq(&registry.get("ecs"), &registry.get("ecs"));
    cr_assert_neq(&registry.get("ecs"), &registry.get("network"));
}

Test(logger_logger_registry, new_logger_has_the_default_level)
{
    LoggerRegistry registry;
    LoggerRegistry debugRegistry(Level::Debug);

    cr_assert(registry.get("ecs").getLevel() == Level::Info);
    cr_assert(debugRegistry.get("ecs").getLevel() == Level::Debug);
}

Test(logger_logger_registry, set_level_changes_every_logger)
{
    LoggerRegistry registry;
    Logger &existing = registry.get("ecs");

    registry.setLevel(Level::Error);
    cr_assert(existing.getLevel() == Level::Error);
    cr_assert(registry.get("network").getLevel() == Level::Error);
}

Test(logger_logger_registry, set_level_of_a_name_changes_only_this_logger)
{
    LoggerRegistry registry;
    Logger &ecs = registry.get("ecs");

    registry.setLevel("network", Level::Debug);
    cr_assert(registry.get("network").getLevel() == Level::Debug);
    cr_assert(ecs.getLevel() == Level::Info);
}

Test(logger_logger_registry, every_logger_writes_to_the_sinks)
{
    LoggerRegistry registry;
    MemorySink &memory = addMemorySink(registry);

    registry.get("ecs").info("from ecs");
    registry.get("network").warn("from network");
    cr_assert_eq(memory.getRecords().size(), 2);
    cr_assert(memory.getRecords()[0].getCategory() == "ecs");
    cr_assert(memory.getRecords()[1].getCategory() == "network");
}

Test(logger_logger_registry, removed_sink_receives_nothing)
{
    LoggerRegistry registry;

    addMemorySink(registry);
    cr_assert(registry.hasSink("memory"));
    cr_assert(registry.removeSink("memory"));
    cr_assert_not(registry.hasSink("memory"));
}

Test(logger_logger_registry, reset_restores_the_levels_and_removes_the_sinks)
{
    LoggerRegistry registry;
    Logger &ecs = registry.get("ecs");

    addMemorySink(registry);
    registry.setLevel(Level::Error);
    registry.reset();
    cr_assert(ecs.getLevel() == Level::Info);
    cr_assert(registry.get("network").getLevel() == Level::Info);
    cr_assert_not(registry.hasSink("memory"));
}

Test(logger_logger_registry, reset_keeps_the_loggers)
{
    LoggerRegistry registry;
    Logger &before = registry.get("ecs");

    registry.reset();
    cr_assert_eq(&registry.get("ecs"), &before);
}

Test(logger_logger_registry, get_from_several_threads)
{
    constexpr int threadCount = 8;
    LoggerRegistry registry;
    std::vector<Logger *> loggers(threadCount, nullptr);
    std::vector<std::thread> threads;

    for (int i = 0; i < threadCount; i++) {
        threads.emplace_back([&registry, &loggers, i] {
            loggers[i] = &registry.get("network");
        });
    }
    for (std::thread &thread : threads) {
        thread.join();
    }
    for (Logger *logger : loggers) {
        cr_assert_eq(logger, loggers.front());
    }
}
