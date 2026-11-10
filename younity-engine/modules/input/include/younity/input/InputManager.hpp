/*
** EPITECH PROJECT, 2026
** younity-engine/modules/input/include/younity/input/InputManager
** File description:
** InputManager header file
*/

#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "younity/input/GamepadAxis.hpp"
#include "younity/input/IInputSource.hpp"
#include "younity/input/Key.hpp"

namespace younity::input {

/**
 * @brief Tells if the actions of the game (e.g. "shoot") are pressed,
 *        from the keys, gamepad buttons and gamepad axes bound to them.
 */
class InputManager
{
public:
    /**
     * @brief A side of a gamepad axis bound to an action.
     */
    struct AxisBinding
    {
        GamepadAxis axis;
        AxisDirection direction;
    };

    /**
     * @brief Everything bound to an action, in the order it was bound.
     */
    struct Bindings
    {
        std::vector<Key> keys;
        std::vector<unsigned int> buttons;
        std::vector<AxisBinding> axes;
    };

    /**
     * @brief Create an input manager reading the real keyboard and gamepads.
     */
    InputManager(void);

    /**
     * @brief Create an input manager reading another source, e.g. a fake one in the tests.
     *
     * @param source The source the state of the devices is read from
     *
     * @throw std::invalid_argument if the source is null.
     */
    explicit InputManager(std::unique_ptr<IInputSource> source);

    /**
     * @brief Bind a key of the keyboard to an action, in addition to what is already bound to it.
     *
     * @param action The name of the action
     * @param key The key pressing the action
     */
    void bindKey(const std::string &action, Key key);

    /**
     * @brief Bind a gamepad button to an action, in addition to what is already bound to it.
     *
     * @param action The name of the action
     * @param button The number of the button pressing the action
     */
    void bindGamepadButton(const std::string &action, unsigned int button);

    /**
     * @brief Bind a side of a gamepad axis to an action, in addition to what is already bound to it.
     *        The action is pressed when the axis is pushed more than halfway on this side.
     *
     * @param action The name of the action
     * @param axis The axis pressing the action
     * @param direction The side of the axis pressing the action
     */
    void bindGamepadAxis(const std::string &action, GamepadAxis axis, AxisDirection direction);

    /**
     * @brief Remove everything bound to an action, e.g. before binding it to another key.
     *
     * @param action The name of the action
     */
    void unbind(const std::string &action);

    /**
     * @brief Read the devices and update the state of the actions, once per frame.
     */
    void update(void);

    /**
     * @brief Check if an action is held down.
     *
     * @param action The name of the action
     *
     * @return true if the action was pressed at the last update, false if nothing is bound to it.
     */
    [[nodiscard]] bool isActionPressed(const std::string &action) const;

    /**
     * @brief Check if an action has just been pressed.
     *
     * @param action The name of the action
     *
     * @return true only at the update where the action went from released to pressed.
     */
    [[nodiscard]] bool isActionJustPressed(const std::string &action) const;

    /**
     * @brief Check if an action has just been released.
     *
     * @param action The name of the action
     *
     * @return true only at the update where the action went from pressed to released.
     */
    [[nodiscard]] bool isActionJustReleased(const std::string &action) const;

    /**
     * @brief Get what is bound to an action, e.g. to show it on a controls screen.
     *
     * @param action The name of the action
     *
     * @return The keys, buttons and axes bound to it, all empty if nothing is.
     */
    [[nodiscard]] Bindings getBindings(const std::string &action) const;

private:
    /**
     * @brief What is bound to an action, and its state at the last two updates.
     */
    struct Action
    {
        std::vector<Key> keys;
        std::vector<unsigned int> buttons;
        std::vector<AxisBinding> axes;
        bool pressed = false;
        bool wasPressed = false;
    };

    /**
     * @brief Check if something bound to the action is held down right now.
     *
     * @param action The action to check
     *
     * @return true if one of its keys, buttons or axes is down.
     */
    [[nodiscard]] bool isDown(const Action &action) const;

    /**
     * @brief Find an action by its name.
     *
     * @param action The name of the action
     *
     * @return The action, or an action never pressed if nothing is bound to this name.
     */
    [[nodiscard]] const Action &getAction(const std::string &action) const;

    std::unique_ptr<IInputSource> _source;
    std::unordered_map<std::string, Action> _actions;
};

} // namespace younity::input
