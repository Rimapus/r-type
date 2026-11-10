/*
** EPITECH PROJECT, 2026
** j-type/client/src/settings/Colorblind
** File description:
** Colorblind source file
*/

#include "settings/Colorblind.hpp"

namespace jtype {

namespace {

// In the order of ColorblindMode
constexpr std::array<std::string_view, COLORBLIND_MODE_COUNT> NAMES{"OFF", "PROTANOPIA", "DEUTERANOPIA", "TRITANOPIA"};

// I + E * (I - S), with S the simulation of the color blindness at full severity and E the matrix of Fidaner et al.
// moving the error to the channels still seen. Under the same simulation, the closest middle shades of the four players
// go from 18.1 to 36.9 Delta E (CIE76) for protanopia, from 10.3 to 26.1 for deuteranopia, from 22.0 to 26.1 for
// tritanopia
constexpr std::array<std::array<float, 9>, COLORBLIND_MODE_COUNT - 1> MATRICES{
    {
     {1.0F, 0.0F, 0.0F, 0.4789F, 0.4769F, 0.0442F, 0.5973F, -0.6887F, 1.0914F},
     {1.0F, 0.0F, 0.0F, 0.1628F, 0.725F, 0.1122F, 0.4547F, -0.6454F, 1.1907F},
     {0.7412F, -0.4072F, 0.666F, 0.0751F, 0.5852F, 0.3397F, 0.0F, 0.0F, 1.0F},
     }
};

} // namespace

std::optional<std::array<float, 9>> getColorblindMatrix(ColorblindMode mode)
{
    if (mode == ColorblindMode::Off) {
        return std::nullopt;
    }
    return MATRICES[static_cast<std::size_t>(mode) - 1];
}

std::string_view getColorblindName(ColorblindMode mode)
{
    return NAMES[static_cast<std::size_t>(mode)];
}

std::optional<ColorblindMode> parseColorblindMode(std::string_view name)
{
    for (std::size_t mode = 0; mode < NAMES.size(); ++mode) {
        if (NAMES[mode] == name) {
            return static_cast<ColorblindMode>(mode);
        }
    }
    return std::nullopt;
}

} // namespace jtype
