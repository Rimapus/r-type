/*
** EPITECH PROJECT, 2026
** younity-engine/modules/input/include/younity/input/SfmlInputSource
** File description:
** SfmlInputSource header file
*/

#pragma once

#include "younity/input/GamepadAxis.hpp"
#include "younity/input/IInputSource.hpp"
#include "younity/input/Key.hpp"

namespace younity::input {

/**
 * @brief Reads the real keyboard and gamepads, with SFML.
 */
class SfmlInputSource : public IInputSource
{
public:
    /**
     * @brief Read the state of the gamepads.
     */
    void update(void) override;

    /**
     * @brief Check if a key of the keyboard is held down.
     *
     * @param key The key to check
     *
     * @return true if the key is down.
     */
    [[nodiscard]] bool isKeyDown(Key key) const override;

    /**
     * @brief Check if a button is held down on any connected gamepad.
     *
     * @param button The number of the button
     *
     * @return true if the button is down.
     */
    [[nodiscard]] bool isGamepadButtonDown(unsigned int button) const override;

    /**
     * @brief Get the position of an axis, from the connected gamepad where it is pushed the most.
     *
     * @param axis The axis to read
     *
     * @return The position between -1 and 1, 0 when the axis is at rest.
     */
    [[nodiscard]] float getGamepadAxis(GamepadAxis axis) const override;
};

} // namespace younity::input
