/*
** EPITECH PROJECT, 2026
** younity-engine/tests/logger/Logger
** File description:
** Logger unit tests
*/

#include <criterion/criterion.h>

#include <format>
#include <source_location>
#include <string>

#include "younity/logger/Level.hpp"
#include "younity/logger/Logger.hpp"
#include "younity/logger/MemorySink.hpp"
#include "younity/logger/Record.hpp"

using younity::logger::Level;
using younity::logger::Logger;
using younity::logger::MemorySink;
using younity::logger::Record;

namespace {

// Counts how many times it is formatted, to check that a filtered message is never formatted
struct Counted
{
    static inline int formatCount = 0;
};

} // namespace

template <>
struct std::formatter<Counted> : std::formatter<std::string_view>
{
    auto format(const Counted &, std::format_context &context) const
    {
        Counted::formatCount++;
        return std::formatter<std::string_view>::format("counted", context);
    }
};

Test(logger_logger, every_method_writes_its_level)
{
    MemorySink sink;
    Logger logger("ecs", sink, Level::Trace);

    logger.trace("message");
    logger.debug("message");
    logger.info("message");
    logger.warn("message");
    logger.error("message");
    logger.critical("message");

    const auto &records = sink.getRecords();

    cr_assert_eq(records.size(), 6);
    cr_assert(records[0].getLevel() == Level::Trace);
    cr_assert(records[1].getLevel() == Level::Debug);
    cr_assert(records[2].getLevel() == Level::Info);
    cr_assert(records[3].getLevel() == Level::Warn);
    cr_assert(records[4].getLevel() == Level::Error);
    cr_assert(records[5].getLevel() == Level::Critical);
}

Test(logger_logger, formats_the_message_with_its_arguments)
{
    MemorySink sink;
    Logger logger("ecs", sink);

    logger.info("Entity {} created at ({}, {})", 4, 1.5, -2);
    cr_assert(sink.getRecords().front().getMessage() == "Entity 4 created at (1.5, -2)");
}

Test(logger_logger, the_category_is_the_logger_name)
{
    MemorySink sink;
    Logger logger("network", sink);

    logger.info("message");
    cr_assert(logger.getName() == "network");
    cr_assert(sink.getRecords().front().getCategory() == "network");
}

Test(logger_logger, captures_the_line_of_the_call)
{
    MemorySink sink;
    Logger logger("ecs", sink);

    const unsigned int line = std::source_location::current().line() + 1;
    logger.error("message");

    const std::source_location &location = sink.getRecords().front().getLocation();

    cr_assert_eq(location.line(), line);
    cr_assert(std::string(location.file_name()).ends_with("Logger.cpp"));
}

Test(logger_logger, records_the_time_of_the_call)
{
    MemorySink sink;
    Logger logger("ecs", sink);

    const Record::Clock::time_point before = Record::Clock::now();
    logger.info("message");
    const Record::Clock::time_point after = Record::Clock::now();

    const Record::Clock::time_point time = sink.getRecords().front().getTime();

    cr_assert(before <= time && time <= after);
}

Test(logger_logger, default_level_is_info)
{
    MemorySink sink;
    const Logger logger("ecs", sink);

    cr_assert(logger.getLevel() == Level::Info);
}

Test(logger_logger, messages_below_the_level_are_filtered)
{
    MemorySink sink;
    Logger logger("ecs", sink, Level::Warn);

    logger.debug("hidden");
    logger.info("hidden");
    logger.warn("shown");
    logger.error("shown");

    cr_assert_eq(sink.getRecords().size(), 2);
    cr_assert(sink.getRecords()[0].getLevel() == Level::Warn);
    cr_assert(sink.getRecords()[1].getLevel() == Level::Error);
}

Test(logger_logger, set_level_changes_the_filter)
{
    MemorySink sink;
    Logger logger("ecs", sink, Level::Error);

    logger.debug("hidden");
    logger.setLevel(Level::Debug);
    logger.debug("shown");

    cr_assert(logger.getLevel() == Level::Debug);
    cr_assert_eq(sink.getRecords().size(), 1);
}

Test(logger_logger, level_off_disables_every_message)
{
    MemorySink sink;
    Logger logger("ecs", sink, Level::Off);

    logger.critical("hidden");
    cr_assert(sink.getRecords().empty());
}

Test(logger_logger, off_is_not_a_message_level)
{
    MemorySink sink;
    Logger logger("ecs", sink, Level::Trace);

    logger.log(Level::Off, "hidden");
    cr_assert_not(logger.isEnabled(Level::Off));
    cr_assert(sink.getRecords().empty());
}

Test(logger_logger, log_writes_the_given_level)
{
    MemorySink sink;
    Logger logger("ecs", sink);

    logger.log(Level::Warn, "Packet {} dropped", 12);
    cr_assert(sink.getRecords().front().getLevel() == Level::Warn);
    cr_assert(sink.getRecords().front().getMessage() == "Packet 12 dropped");
}

Test(logger_logger, filtered_message_is_not_formatted)
{
    MemorySink sink;
    Logger logger("ecs", sink, Level::Warn);

    Counted::formatCount = 0;
    logger.debug("{}", Counted{});
    cr_assert_eq(Counted::formatCount, 0);
    logger.warn("{}", Counted{});
    cr_assert_eq(Counted::formatCount, 1);
}
