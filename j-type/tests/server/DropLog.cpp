/*
** EPITECH PROJECT, 2026
** j-type/tests/server/DropLog
** File description:
** Unit tests of the rate-limited report of the rejected datagrams
*/

#include <criterion/criterion.h>

#include <chrono>
#include <string>

#include "DropLog.hpp"

using jtype::DropLog;

namespace {

// A time far enough from the epoch that the first report is due
const std::chrono::steady_clock::time_point START =
    std::chrono::steady_clock::time_point{} + std::chrono::seconds(10);

} // namespace

Test(server_drop_log, first_rejection_is_reported)
{
    DropLog log;

    cr_assert_eq(log.reject("truncated", START), "Dropped 1 invalid datagrams, e.g. truncated");
}

Test(server_drop_log, rejections_are_counted_until_the_next_report)
{
    DropLog log;

    // The first rejection is reported at once, then the interval starts
    (void)log.reject("truncated", START);
    cr_assert(log.reject("invalid value", START + std::chrono::milliseconds(500)).empty());
    // The report keeps the reason of the first rejection of the interval
    cr_assert_eq(log.reject("bad magic", START + std::chrono::seconds(1)),
                 "Dropped 2 invalid datagrams, e.g. invalid value");
}

Test(server_drop_log, a_report_resets_the_count)
{
    DropLog log;

    (void)log.reject("truncated", START);
    (void)log.reject("truncated", START + std::chrono::seconds(1));
    cr_assert_eq(log.reject("bad magic", START + std::chrono::seconds(2)),
                 "Dropped 1 invalid datagrams, e.g. bad magic");
}

Test(server_drop_log, a_longer_interval_waits)
{
    DropLog log(std::chrono::seconds(5));

    (void)log.reject("truncated", START);
    cr_assert(log.reject("truncated", START + std::chrono::seconds(4)).empty());
    cr_assert_eq(log.reject("truncated", START + std::chrono::seconds(5)),
                 "Dropped 2 invalid datagrams, e.g. truncated");
}
