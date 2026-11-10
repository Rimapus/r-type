/*
** EPITECH PROJECT, 2026
** younity-engine/tests/logger/SinkDispatcher
** File description:
** SinkDispatcher unit tests
*/

#include <criterion/criterion.h>

#include <memory>
#include <source_location>
#include <stdexcept>
#include <thread>
#include <utility>
#include <vector>

#include "younity/logger/Level.hpp"
#include "younity/logger/MemorySink.hpp"
#include "younity/logger/Record.hpp"
#include "younity/logger/SinkDispatcher.hpp"

using younity::logger::Level;
using younity::logger::MemorySink;
using younity::logger::Record;
using younity::logger::SinkDispatcher;

namespace {

Record makeRecord(const char *message)
{
    return Record(Level::Info, "ecs", message, std::source_location::current(), Record::Clock::now());
}

// Adds a memory sink to the dispatcher, and keeps access to it to read the records
MemorySink &addMemorySink(SinkDispatcher &dispatcher, const char *name)
{
    auto sink = std::make_unique<MemorySink>();
    MemorySink &memory = *sink;

    dispatcher.addSink(name, std::move(sink));
    return memory;
}

} // namespace

Test(logger_sink_dispatcher, writes_to_every_sink)
{
    SinkDispatcher dispatcher;
    MemorySink &first = addMemorySink(dispatcher, "first");
    MemorySink &second = addMemorySink(dispatcher, "second");

    dispatcher.write(makeRecord("message"));
    cr_assert_eq(first.getRecords().size(), 1);
    cr_assert_eq(second.getRecords().size(), 1);
}

Test(logger_sink_dispatcher, without_sink_writes_nothing)
{
    SinkDispatcher dispatcher;

    dispatcher.write(makeRecord("message"));
    cr_assert_not(dispatcher.hasSink("memory"));
}

Test(logger_sink_dispatcher, has_the_added_sinks)
{
    SinkDispatcher dispatcher;

    addMemorySink(dispatcher, "memory");
    cr_assert(dispatcher.hasSink("memory"));
    cr_assert_not(dispatcher.hasSink("file"));
}

Test(logger_sink_dispatcher, removed_sink_receives_nothing)
{
    SinkDispatcher dispatcher;
    MemorySink &kept = addMemorySink(dispatcher, "kept");

    addMemorySink(dispatcher, "removed");
    cr_assert(dispatcher.removeSink("removed"));
    dispatcher.write(makeRecord("message"));
    cr_assert_not(dispatcher.hasSink("removed"));
    cr_assert_eq(kept.getRecords().size(), 1);
}

Test(logger_sink_dispatcher, remove_a_missing_sink_does_nothing)
{
    SinkDispatcher dispatcher;

    cr_assert_not(dispatcher.removeSink("missing"));
}

Test(logger_sink_dispatcher, add_a_sink_with_the_same_name_replaces_it)
{
    SinkDispatcher dispatcher;

    // The replaced sink is destroyed, only the new one can be checked
    addMemorySink(dispatcher, "memory");
    MemorySink &current = addMemorySink(dispatcher, "memory");

    dispatcher.write(makeRecord("message"));
    cr_assert(dispatcher.hasSink("memory"));
    cr_assert_eq(current.getRecords().size(), 1);
}

Test(logger_sink_dispatcher, add_a_null_sink_throws)
{
    SinkDispatcher dispatcher;

    cr_assert_throw(dispatcher.addSink("null", nullptr), std::invalid_argument);
}

Test(logger_sink_dispatcher, clear_removes_every_sink)
{
    SinkDispatcher dispatcher;

    addMemorySink(dispatcher, "first");
    addMemorySink(dispatcher, "second");
    dispatcher.clear();
    cr_assert_not(dispatcher.hasSink("first"));
    cr_assert_not(dispatcher.hasSink("second"));
}

Test(logger_sink_dispatcher, writes_from_several_threads)
{
    constexpr int threadCount = 8;
    constexpr int messageCount = 1000;
    SinkDispatcher dispatcher;
    MemorySink &memory = addMemorySink(dispatcher, "memory");
    std::vector<std::thread> threads;

    for (int i = 0; i < threadCount; i++) {
        threads.emplace_back([&dispatcher] {
            for (int j = 0; j < messageCount; j++) {
                dispatcher.write(makeRecord("message"));
            }
        });
    }
    for (std::thread &thread : threads) {
        thread.join();
    }
    cr_assert_eq(memory.getRecords().size(), threadCount * messageCount);
}
