/*
** EPITECH PROJECT, 2026
** j-type/client/src/input/Controls
** File description:
** Controls header file
*/

#pragma once

#include <cstdint>
#include <memory>
#include <optional>

#include "input/Bindings.hpp"
#include "younity/input/IInputSource.hpp"
#include "younity/input/InputManager.hpp"
#include "younity/input/Key.hpp"

namespace jtype {

/**
 * @brief An action of the menus.
 */
enum class MenuAction {
    Up,
    Down,
    Left,
    Right,
    Confirm,
    Back
};

/**
 * @brief The keys and the gamepad inputs of the player: the actions of the game follow the bindings of the settings.
 *        In the menus, the arrows or the left stick of a gamepad move, Enter or its first button confirms, Escape or
 *        its second button goes back, whatever the bindings.
 */
class Controls
{
public:
    /**
     * @brief Bind the actions of the game to their default keys, and the actions of the menus.
     */
    Controls(void);

    /**
     * @brief Bind the actions like Controls(void), read from another source, e.g. a fake one in the tests.
     *
     * @param source The source the state of the keyboard and the gamepads is read from
     */
    explicit Controls(std::unique_ptr<younity::input::IInputSource> source);

    /**
     * @brief Bind the actions of the game to the keys and the gamepad inputs of the player.
     *
     * @param bindings The bindings, e.g. of the settings
     */
    void apply(const Bindings &bindings);

    /**
     * @brief Read the keyboard and the gamepads, once per step, before the other functions.
     */
    void update(void);

    /**
     * @brief Give the keyboard and the gamepads to the game, or take them away, e.g. while the editor of the engine has
     *        them: no action is then pressed and no key is found to bind. Enabled at the creation.
     *
     * @param isEnabled true to give them to the game
     */
    void setEnabled(bool isEnabled) noexcept;

    /**
     * @brief Read the actions held down.
     *
     * @return The input bitmask, made of the bits of jtype::input.
     */
    [[nodiscard]] std::uint8_t read(void) const;

    /**
     * @brief Check if the mute key was just pressed, so holding it switches the sound once.
     *
     * @return true on the step the key goes down.
     */
    [[nodiscard]] bool isMutePressed(void) const;

    /**
     * @brief Check if an action of the menus was just pressed, so holding it moves one line at a time.
     *
     * @param action The action
     *
     * @return true on the step it goes down.
     */
    [[nodiscard]] bool isMenuPressed(MenuAction action) const;

    /**
     * @brief Find a key that was just pressed, to bind it to an action.
     *
     * @return The key, or nothing if no key went down at this step.
     */
    [[nodiscard]] std::optional<younity::input::Key> getKeyJustPressed(void) const;

    /**
     * @brief Find a gamepad button that was just pressed, or an axis just pushed past halfway, to bind it to an action.
     *
     * @return The input, or nothing if none was at this step.
     */
    [[nodiscard]] std::optional<GamepadInput> getGamepadJustPressed(void) const;

private:
    younity::input::InputManager _inputs;
    bool _isEnabled = true;
};

} // namespace jtype
