/*
** EPITECH PROJECT, 2026
** j-type/tests/common/Difficulty
** File description:
** Difficulty unit tests
*/

#include <criterion/criterion.h>

#include <optional>

#include "jtype/common/Difficulty.hpp"
#include "jtype/common/Gameplay.hpp"

using jtype::Difficulty;
using jtype::DifficultyRules;

Test(common_difficulty, normal_keeps_the_values_of_the_game)
{
    const DifficultyRules normal = jtype::getDifficultyRules(Difficulty::Normal);

    cr_assert_eq(normal.lives, jtype::PLAYER_LIVES);
    cr_assert_float_eq(normal.shotSpeed, 1.0F, 1e-6F);
    cr_assert_float_eq(normal.shootDelay, 1.0F, 1e-6F);
}

Test(common_difficulty, easy_gives_more_lives_and_fewer_slower_shots)
{
    const DifficultyRules easy = jtype::getDifficultyRules(Difficulty::Easy);

    cr_assert_gt(easy.lives, jtype::PLAYER_LIVES);
    cr_assert_leq(easy.lives, jtype::MAX_LIVES);
    cr_assert_lt(easy.shotSpeed, 1.0F);
    cr_assert_gt(easy.shootDelay, 1.0F);
}

Test(common_difficulty, hard_gives_fewer_lives_and_more_faster_shots)
{
    const DifficultyRules hard = jtype::getDifficultyRules(Difficulty::Hard);

    cr_assert_lt(hard.lives, jtype::PLAYER_LIVES);
    cr_assert_gt(hard.lives, 0);
    cr_assert_gt(hard.shotSpeed, 1.0F);
    cr_assert_lt(hard.shootDelay, 1.0F);
}

Test(common_difficulty, is_read_and_written_by_its_name)
{
    for (const Difficulty difficulty : {Difficulty::Easy, Difficulty::Normal, Difficulty::Hard}) {
        cr_assert(jtype::parseDifficulty(jtype::getDifficultyName(difficulty)) == difficulty);
    }
    cr_assert_str_eq(jtype::getDifficultyName(Difficulty::Hard).data(), "hard");
    cr_assert_not(jtype::parseDifficulty("Easy").has_value());
    cr_assert_not(jtype::parseDifficulty("").has_value());
}
