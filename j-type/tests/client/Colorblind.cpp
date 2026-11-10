/*
** EPITECH PROJECT, 2026
** j-type/tests/client/Colorblind
** File description:
** Unit tests of the color corrections for the color blindnesses
*/

#include <criterion/criterion.h>

#include <array>
#include <cstddef>
#include <optional>

#include "settings/Colorblind.hpp"

using jtype::ColorblindMode;

Test(client_colorblind, off_keeps_the_colors)
{
    cr_assert_not(jtype::getColorblindMatrix(ColorblindMode::Off).has_value());
}

Test(client_colorblind, every_correction_keeps_the_grays)
{
    for (std::size_t index = 1; index < jtype::COLORBLIND_MODE_COUNT; ++index) {
        const std::optional<std::array<float, 9>> matrix = jtype::getColorblindMatrix(static_cast<ColorblindMode>(index));

        cr_assert(matrix.has_value());
        // A gray has the same red, green and blue: a row summing to 1 gives it back
        for (std::size_t row = 0; row < 3; ++row) {
            cr_assert_float_eq((*matrix)[row * 3] + (*matrix)[row * 3 + 1] + (*matrix)[row * 3 + 2], 1.0F, 0.001F);
        }
    }
}

Test(client_colorblind, the_corrections_differ)
{
    cr_assert(jtype::getColorblindMatrix(ColorblindMode::Protanopia) != jtype::getColorblindMatrix(ColorblindMode::Deuteranopia));
    cr_assert(jtype::getColorblindMatrix(ColorblindMode::Deuteranopia) != jtype::getColorblindMatrix(ColorblindMode::Tritanopia));
}

Test(client_colorblind, every_mode_has_a_name_read_back)
{
    for (std::size_t index = 0; index < jtype::COLORBLIND_MODE_COUNT; ++index) {
        const auto mode = static_cast<ColorblindMode>(index);

        cr_assert(jtype::parseColorblindMode(jtype::getColorblindName(mode)) == mode);
    }
    cr_assert(jtype::getColorblindName(ColorblindMode::Deuteranopia) == "DEUTERANOPIA");
    cr_assert_not(jtype::parseColorblindMode("DALTONISM").has_value());
}
