/*
** EPITECH PROJECT, 2026
** j-type/common/include/jtype/common/Difficulty
** File description:
** Difficulty header file
*/

#pragma once

#include <cstdint>
#include <optional>
#include <string_view>

#include "jtype/common/Gameplay.hpp"

namespace jtype {

/**
 * @brief How hard the game is, chosen on the server: it changes the lives of the players and the shots of the Bydos.
 */
enum class Difficulty {
    Easy,
    Normal,
    Hard
};

/**
 * @brief What a difficulty changes in the values of Gameplay.hpp.
 */
struct DifficultyRules
{
    // The lives of a player when the game starts, at most MAX_LIVES
    std::uint8_t lives = PLAYER_LIVES;
    // Multiplies ENEMY_MISSILE_SPEED, for the Bydos and the boss
    float shotSpeed = 1.0F;
    // Multiplies ENEMY_SHOOT_DELAY: above 1, the Bydos shoot less often
    float shootDelay = 1.0F;
};

// The rules of Difficulty::Normal: the values of Gameplay.hpp as they are
inline constexpr DifficultyRules NORMAL_RULES{};

/**
 * @brief Get what a difficulty changes, e.g. Easy gives 5 lives and slower shots, less often.
 *
 * @param difficulty The difficulty
 *
 * @return Its rules.
 */
[[nodiscard]] DifficultyRules getDifficultyRules(Difficulty difficulty) noexcept;

/**
 * @brief Get the name of a difficulty, as the server reads it.
 *
 * @param difficulty The difficulty
 *
 * @return "easy", "normal" or "hard".
 */
[[nodiscard]] std::string_view getDifficultyName(Difficulty difficulty) noexcept;

/**
 * @brief Read the name of a difficulty.
 *
 * @param name "easy", "normal" or "hard"
 *
 * @return The difficulty, or nothing for another name.
 */
[[nodiscard]] std::optional<Difficulty> parseDifficulty(std::string_view name) noexcept;

} // namespace jtype
