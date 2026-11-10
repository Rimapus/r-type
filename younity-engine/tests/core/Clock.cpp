/*
** EPITECH PROJECT, 2026
** younity-engine/tests/core/Clock
** File description:
** Clock unit tests
*/

#include <criterion/criterion.h>

#include <chrono>

#include "younity/core/Clock.hpp"

namespace {

// Clock moved by hand by the tests, so they never sleep and their results are exact
struct FakeSource
{
    using duration = std::chrono::nanoseconds;
    using rep = duration::rep;
    using period = duration::period;
    using time_point = std::chrono::time_point<FakeSource>;

    // Named by the std::chrono clock requirements
    static constexpr bool is_steady = true; // NOLINT(readability-identifier-naming)

    // Criterion runs each test in its own process: this time is never shared between tests
    static inline time_point current{};

    static time_point now(void) noexcept
    {
        return current;
    }

    static void advance(std::chrono::nanoseconds elapsed) noexcept
    {
        current += elapsed;
    }
};

using FakeClock = younity::BasicClock<FakeSource>;

static_assert(younity::TimeSource<std::chrono::steady_clock>);
// The wall clock jumps when the date of the computer changes
static_assert(!younity::TimeSource<std::chrono::system_clock>);

} // namespace

Test(core_clock, starts_at_zero)
{
    const FakeClock clock;

    cr_assert_float_eq(clock.getElapsedTime(), 0.0F, 0.0001F);
}

Test(core_clock, elapsed_time_is_in_seconds)
{
    const FakeClock clock;

    FakeSource::advance(std::chrono::milliseconds(1500));
    cr_assert_float_eq(clock.getElapsedTime(), 1.5F, 0.0001F);
}

Test(core_clock, elapsed_time_does_not_restart)
{
    const FakeClock clock;

    FakeSource::advance(std::chrono::seconds(1));
    (void)clock.getElapsedTime();
    FakeSource::advance(std::chrono::seconds(1));
    cr_assert_float_eq(clock.getElapsedTime(), 2.0F, 0.0001F);
}

Test(core_clock, restart_returns_the_delta_time)
{
    FakeClock clock;

    FakeSource::advance(std::chrono::milliseconds(16));
    cr_assert_float_eq(clock.restart(), 0.016F, 0.0001F);
}

Test(core_clock, restart_measures_from_the_last_restart)
{
    FakeClock clock;

    FakeSource::advance(std::chrono::milliseconds(100));
    (void)clock.restart();
    FakeSource::advance(std::chrono::milliseconds(25));
    cr_assert_float_eq(clock.restart(), 0.025F, 0.0001F);
    cr_assert_float_eq(clock.getElapsedTime(), 0.0F, 0.0001F);
}

Test(core_clock, real_clock_moves_forward)
{
    const younity::Clock clock;
    const float first = clock.getElapsedTime();

    cr_assert_geq(first, 0.0F);
    cr_assert_geq(clock.getElapsedTime(), first);
}
