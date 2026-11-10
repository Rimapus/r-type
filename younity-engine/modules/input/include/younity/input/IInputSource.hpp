/*
** EPITECH PROJECT, 2026
** younity-engine/modules/input/include/younity/input/IInputSource
** File description:
** IInputSource header file
*/

#pragma once

#include "younity/input/GamepadAxis.hpp"
#include "younity/input/Key.hpp"

namespace younity::input {

/**
 * @brief Where the InputManager reads the state of the keyboard and of the gamepads.
 */
class IInputSource
{
public:
    virtual ~IInputSource(void) = default;

    /**
     * @brief Read the state of the devices, called once per frame before the other functions.
     */
    virtual void update(void) = 0;

    /**
     * @brief Check if a key of the keyboard is held down.
     *
     * @param key The key to check
     *
     * @return true if the key is down.
     */
    [[nodiscard]] virtual bool isKeyDown(Key key) const = 0;

    /**
     * @brief Check if a button is held down on any connected gamepad.
     *
     * @param button The number of the button
     *
     * @return true if the button is down.
     */
    [[nodiscard]] virtual bool isGamepadButtonDown(unsigned int button) const = 0;

    /**
     * @brief Get the position of an axis, from the connected gamepad where it is pushed the most.
     *
     * @param axis The axis to read
     *
     * @return The position between -1 and 1, 0 when the axis is at rest.
     */
    [[nodiscard]] virtual float getGamepadAxis(GamepadAxis axis) const = 0;
};

} // namespace younity::input
