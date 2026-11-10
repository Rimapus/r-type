/*
** EPITECH PROJECT, 2026
** younity-engine/tests/logger/MemorySink
** File description:
** MemorySink unit tests
*/

#include <criterion/criterion.h>

#include <source_location>

#include "younity/logger/ISink.hpp"
#include "younity/logger/Level.hpp"
#include "younity/logger/MemorySink.hpp"
#include "younity/logger/Record.hpp"

using younity::logger::ISink;
using younity::logger::Level;
using younity::logger::MemorySink;
using younity::logger::Record;

namespace {

Record makeRecord(Level level, const char *message)
{
    return Record(level, "ecs", message, std::source_location::current(), Record::Clock::now());
}

} // namespace

Test(logger_memory_sink, starts_empty)
{
    const MemorySink sink;

    cr_assert(sink.getRecords().empty());
}

Test(logger_memory_sink, keeps_the_records_in_order)
{
    MemorySink sink;

    sink.write(makeRecord(Level::Info, "first"));
    sink.write(makeRecord(Level::Warn, "second"));
    cr_assert_eq(sink.getRecords().size(), 2);
    cr_assert(sink.getRecords()[0].getMessage() == "first");
    cr_assert(sink.getRecords()[1].getMessage() == "second");
}

Test(logger_memory_sink, keeps_every_field_of_the_record)
{
    MemorySink sink;
    const Record record = makeRecord(Level::Error, "Texture not found");

    sink.write(record);

    const Record &kept = sink.getRecords().front();

    cr_assert(kept.getLevel() == Level::Error);
    cr_assert(kept.getCategory() == "ecs");
    cr_assert(kept.getMessage() == "Texture not found");
    cr_assert_eq(kept.getLocation().line(), record.getLocation().line());
    cr_assert(kept.getTime() == record.getTime());
}

Test(logger_memory_sink, is_usable_through_the_sink_interface)
{
    MemorySink sink;
    ISink &base = sink;

    base.write(makeRecord(Level::Debug, "through ISink"));
    cr_assert_eq(sink.getRecords().size(), 1);
}

Test(logger_memory_sink, clear_removes_the_records)
{
    MemorySink sink;

    sink.write(makeRecord(Level::Info, "message"));
    sink.clear();
    cr_assert(sink.getRecords().empty());
}
