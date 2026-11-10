/*
** EPITECH PROJECT, 2026
** younity-engine/modules/input/include/younity/input/GamepadAxis
** File description:
** GamepadAxis header file
*/

#pragma once

namespace younity::input {

/**
 * @brief Axes of a gamepad that can be bound to an action.
 *        X and Y are the left stick, PovX and PovY the directional pad on most gamepads:
 *        the other ones depend on the gamepad.
 */
enum class GamepadAxis {
    X,
    Y,
    Z,
    R,
    U,
    V,
    PovX,
    PovY,
};

/**
 * @brief Side of a gamepad axis that triggers an action, e.g. Negative on X is the left of the stick.
 */
enum class AxisDirection {
    Negative,
    Positive,
};

} // namespace younity::input
