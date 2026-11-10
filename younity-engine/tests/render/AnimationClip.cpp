/*
** EPITECH PROJECT, 2026
** younity-engine/tests/render/AnimationClip
** File description:
** AnimationClip unit tests
*/

#include <criterion/criterion.h>

#include <limits>
#include <stdexcept>

#include "younity/render/AnimationClip.hpp"

using younity::Recti;
using younity::render::AnimationClip;
using younity::render::GridLayout;

Test(render_animation_clip, grid_frames_are_read_left_to_right)
{
    const AnimationClip clip = AnimationClip::fromGrid(GridLayout{
                                                           .frameSize = {16, 8},
                                                           .columns = 4,
                                                           .first = 0,
                                                           .count = 3
    },
                                                       0.1F);

    cr_assert_eq(clip.frames.size(), 3U);
    cr_assert(clip.frames[0] == (Recti{0, 0, 16, 8}));
    cr_assert(clip.frames[1] == (Recti{16, 0, 16, 8}));
    cr_assert(clip.frames[2] == (Recti{32, 0, 16, 8}));
}

Test(render_animation_clip, grid_frames_go_to_the_next_row)
{
    const AnimationClip clip = AnimationClip::fromGrid(GridLayout{
                                                           .frameSize = {16, 8},
                                                           .columns = 2,
                                                           .first = 1,
                                                           .count = 3
    },
                                                       0.1F);

    cr_assert(clip.frames[0] == (Recti{16, 0, 16, 8}));
    cr_assert(clip.frames[1] == (Recti{0, 8, 16, 8}));
    cr_assert(clip.frames[2] == (Recti{16, 8, 16, 8}));
}

Test(render_animation_clip, grid_keeps_the_duration_and_the_loop)
{
    const AnimationClip clip = AnimationClip::fromGrid(GridLayout{
                                                           .frameSize = {8, 8},
                                                           .columns = 1,
                                                           .first = 0,
                                                           .count = 1
    },
                                                       0.25F, false);

    cr_assert_float_eq(clip.frameDuration, 0.25F, 0.00001F);
    cr_assert_not(clip.loop);
}

Test(render_animation_clip, grid_loops_by_default)
{
    const AnimationClip clip = AnimationClip::fromGrid(GridLayout{
                                                           .frameSize = {8, 8},
                                                           .columns = 1,
                                                           .first = 0,
                                                           .count = 1
    },
                                                       0.1F);

    cr_assert(clip.loop);
}

Test(render_animation_clip, invalid_grid_throws)
{
    cr_assert_throw((void)AnimationClip::fromGrid(GridLayout{
                                                      .frameSize = {0, 8},
                                                      .columns = 1,
                                                      .first = 0,
                                                      .count = 1
    },
                                                  0.1F),
                    std::invalid_argument);
    cr_assert_throw((void)AnimationClip::fromGrid(GridLayout{
                                                      .frameSize = {8, -1},
                                                      .columns = 1,
                                                      .first = 0,
                                                      .count = 1
    },
                                                  0.1F),
                    std::invalid_argument);
    cr_assert_throw((void)AnimationClip::fromGrid(GridLayout{
                                                      .frameSize = {8, 8},
                                                      .columns = 0,
                                                      .first = 0,
                                                      .count = 1
    },
                                                  0.1F),
                    std::invalid_argument);
    cr_assert_throw((void)AnimationClip::fromGrid(GridLayout{
                                                      .frameSize = {8, 8},
                                                      .columns = 1,
                                                      .first = -1,
                                                      .count = 1
    },
                                                  0.1F),
                    std::invalid_argument);
    cr_assert_throw((void)AnimationClip::fromGrid(GridLayout{
                                                      .frameSize = {8, 8},
                                                      .columns = 1,
                                                      .first = 0,
                                                      .count = 0
    },
                                                  0.1F),
                    std::invalid_argument);
}

Test(render_animation_clip, invalid_duration_throws)
{
    const GridLayout grid{
        .frameSize = {8, 8},
        .columns = 1,
        .first = 0,
        .count = 1
    };

    cr_assert_throw((void)AnimationClip::fromGrid(grid, 0.0F), std::invalid_argument);
    cr_assert_throw((void)AnimationClip::fromGrid(grid, -0.1F), std::invalid_argument);
    cr_assert_throw((void)AnimationClip::fromGrid(grid, std::numeric_limits<float>::quiet_NaN()), std::invalid_argument);
    cr_assert_throw((void)AnimationClip::fromGrid(grid, std::numeric_limits<float>::infinity()), std::invalid_argument);
}

Test(render_animation_clip, grid_beyond_the_int_range_throws)
{
    constexpr int big = std::numeric_limits<int>::max();

    // first + count would overflow
    cr_assert_throw((void)AnimationClip::fromGrid(GridLayout{
                                                      .frameSize = {8, 8},
                                                      .columns = 1,
                                                      .first = big,
                                                      .count = 2
    },
                                                  0.1F),
                    std::invalid_argument);
    // the right of the last column would overflow
    cr_assert_throw((void)AnimationClip::fromGrid(GridLayout{
                                                      .frameSize = {big / 2, 8},
                                                      .columns = 3,
                                                      .first = 0,
                                                      .count = 3
    },
                                                  0.1F),
                    std::invalid_argument);
    // the bottom of the last row would overflow
    cr_assert_throw((void)AnimationClip::fromGrid(GridLayout{
                                                      .frameSize = {8, big / 2},
                                                      .columns = 1,
                                                      .first = 0,
                                                      .count = 3
    },
                                                  0.1F),
                    std::invalid_argument);
}

Test(render_animation_clip, grid_at_the_edge_of_the_int_range_is_accepted)
{
    constexpr int big = std::numeric_limits<int>::max();
    const AnimationClip clip = AnimationClip::fromGrid(GridLayout{
                                                           .frameSize = {big / 2, 1},
                                                           .columns = 2,
                                                           .first = 0,
                                                           .count = 2
    },
                                                       0.1F);

    cr_assert_eq(clip.frames[1].left, big / 2);
}
