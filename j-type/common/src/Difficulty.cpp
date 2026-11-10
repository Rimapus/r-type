/*
** EPITECH PROJECT, 2026
** j-type/common/src/Difficulty
** File description:
** Difficulty source file
*/

#include "jtype/common/Difficulty.hpp"

#include <array>
#include <cstddef>

namespace jtype {

namespace {

// By Difficulty, in its order
constexpr std::array<std::string_view, 3> DIFFICULTY_NAMES{"easy", "normal", "hard"};

// Easy plays with every life an extra life can bring, and shots 30% slower, two thirds as often
constexpr DifficultyRules EASY_RULES{MAX_LIVES, 0.7F, 1.5F};
constexpr DifficultyRules HARD_RULES{PLAYER_LIVES - 1, 1.25F, 0.75F};

} // namespace

DifficultyRules getDifficultyRules(Difficulty difficulty) noexcept
{
    switch (difficulty) {
    case Difficulty::Easy:
        return EASY_RULES;
    case Difficulty::Hard:
        return HARD_RULES;
    case Difficulty::Normal:
        break;
    }
    return NORMAL_RULES;
}

std::string_view getDifficultyName(Difficulty difficulty) noexcept
{
    return DIFFICULTY_NAMES[static_cast<std::size_t>(difficulty)];
}

std::optional<Difficulty> parseDifficulty(std::string_view name) noexcept
{
    for (std::size_t index = 0; index < DIFFICULTY_NAMES.size(); ++index) {
        if (DIFFICULTY_NAMES[index] == name) {
            return static_cast<Difficulty>(index);
        }
    }
    return std::nullopt;
}

} // namespace jtype
