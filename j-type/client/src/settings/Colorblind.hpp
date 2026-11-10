/*
** EPITECH PROJECT, 2026
** j-type/client/src/settings/Colorblind
** File description:
** Colorblind header file
*/

#pragma once

#include <array>
#include <cstddef>
#include <optional>
#include <string_view>

namespace jtype {

/**
 * @brief The color blindness the colors of the window are corrected for.
 */
enum class ColorblindMode {
    Off,
    // Red seen dark and mixed up with green
    Protanopia,
    // Green mixed up with red
    Deuteranopia,
    // Blue mixed up with green, yellow with pink
    Tritanopia
};

inline constexpr std::size_t COLORBLIND_MODE_COUNT = 4;

/**
 * @brief Get the matrix that corrects the colors of the window for a color blindness, as younity::render::ColorMatrix
 *        takes it: the daltonization of Fidaner, Lin and Ozguven (2005) built on the simulation of Machado, Oliveira
 *        and Fernandes (2009). The colors lost to the color blindness are moved to the channels still seen, and the
 *        grays, white included, stay the same.
 *
 * @param mode The color blindness
 *
 * @return The matrix, written row by row, or nothing for Off.
 */
[[nodiscard]] std::optional<std::array<float, 9>> getColorblindMatrix(ColorblindMode mode);

/**
 * @brief Get the name of a mode, as the settings screen shows it and the settings file stores it.
 *
 * @param mode The mode
 *
 * @return Its name, e.g. "DEUTERANOPIA".
 */
[[nodiscard]] std::string_view getColorblindName(ColorblindMode mode);

/**
 * @brief Find a mode by its name.
 *
 * @param name The name given by getColorblindName()
 *
 * @return The mode, or nothing for an unknown name.
 */
[[nodiscard]] std::optional<ColorblindMode> parseColorblindMode(std::string_view name);

} // namespace jtype
