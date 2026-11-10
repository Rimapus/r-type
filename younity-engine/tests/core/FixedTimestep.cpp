/*
** EPITECH PROJECT, 2026
** younity-engine/tests/core/FixedTimestep
** File description:
** FixedTimestep unit tests
*/

#include <criterion/criterion.h>

#include <cstddef>
#include <limits>
#include <stdexcept>

#include "younity/core/FixedTimestep.hpp"

using younity::FixedTimestep;

Test(core_fixed_timestep, keeps_its_step)
{
    const FixedTimestep timestep(0.02F);

    cr_assert_float_eq(timestep.getStep(), 0.02F, 0.00001F);
}

Test(core_fixed_timestep, short_frame_runs_no_step)
{
    FixedTimestep timestep(0.1F);

    cr_assert_eq(timestep.advance(0.05F), 0U);
}

Test(core_fixed_timestep, frame_of_one_step_runs_one_step)
{
    FixedTimestep timestep(0.1F);

    cr_assert_eq(timestep.advance(0.1F), 1U);
}

Test(core_fixed_timestep, long_frame_runs_several_steps)
{
    FixedTimestep timestep(0.1F);

    cr_assert_eq(timestep.advance(0.2F), 2U);
}

Test(core_fixed_timestep, leftover_time_is_kept_for_the_next_frame)
{
    FixedTimestep timestep(0.1F);

    cr_assert_eq(timestep.advance(0.06F), 0U);
    cr_assert_eq(timestep.advance(0.06F), 1U);
}

Test(core_fixed_timestep, steps_match_the_total_time)
{
    FixedTimestep timestep(1.0F / 60.0F);
    std::size_t steps = 0;

    // One second at 144 frames per second must run 60 steps at 60 Hz, give or take the leftover
    for (int frame = 0; frame < 144; ++frame) {
        steps += timestep.advance(1.0F / 144.0F);
    }
    cr_assert(steps == 59U || steps == 60U, "got %zu steps", steps);
}

Test(core_fixed_timestep, alpha_is_the_part_of_the_next_step)
{
    FixedTimestep timestep(0.1F);

    (void)timestep.advance(0.125F);
    cr_assert_float_eq(timestep.getAlpha(), 0.25F, 0.0001F);
}

Test(core_fixed_timestep, alpha_stays_below_one)
{
    FixedTimestep timestep(0.1F);

    (void)timestep.advance(0.1F);
    cr_assert_float_eq(timestep.getAlpha(), 0.0F, 0.0001F);
}

Test(core_fixed_timestep, long_frame_is_clamped)
{
    FixedTimestep timestep(0.1F, 0.25F);

    // A 10 seconds freeze (e.g. a breakpoint) must not run 100 steps to catch up
    cr_assert_eq(timestep.advance(10.0F), 2U);
}

Test(core_fixed_timestep, invalid_frame_time_runs_no_step)
{
    FixedTimestep timestep(0.1F);

    cr_assert_eq(timestep.advance(-1.0F), 0U);
    cr_assert_eq(timestep.advance(std::numeric_limits<float>::quiet_NaN()), 0U);
    cr_assert_float_eq(timestep.getAlpha(), 0.0F, 0.0001F);
}

Test(core_fixed_timestep, reset_drops_the_leftover_time)
{
    FixedTimestep timestep(0.1F);

    (void)timestep.advance(0.09F);
    timestep.reset();
    cr_assert_float_eq(timestep.getAlpha(), 0.0F, 0.0001F);
    cr_assert_eq(timestep.advance(0.02F), 0U);
}

Test(core_fixed_timestep, invalid_step_throws)
{
    cr_assert_throw(FixedTimestep timestep(0.0F), std::invalid_argument);
    cr_assert_throw(FixedTimestep timestep(-0.1F), std::invalid_argument);
    cr_assert_throw(FixedTimestep timestep(std::numeric_limits<float>::infinity()), std::invalid_argument);
}

Test(core_fixed_timestep, max_frame_time_shorter_than_a_step_throws)
{
    // No step could ever run
    cr_assert_throw(FixedTimestep timestep(0.1F, 0.05F), std::invalid_argument);
}
