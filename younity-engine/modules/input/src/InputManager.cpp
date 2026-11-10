/*
** EPITECH PROJECT, 2026
** younity-engine/modules/input/src/InputManager
** File description:
** InputManager source file
*/

#include "younity/input/InputManager.hpp"
#include "younity/input/SfmlInputSource.hpp"
#include <stdexcept>
#include <utility>

namespace younity::input {

namespace {

// An axis presses its action when it is pushed more than halfway
constexpr float AXIS_THRESHOLD = 0.5F;

} // namespace

InputManager::InputManager(void)
    : _source(std::make_unique<SfmlInputSource>()) {}

InputManager::InputManager(std::unique_ptr<IInputSource> source)
    : _source(std::move(source))
{
    if (this->_source == nullptr) {
        throw std::invalid_argument("younity::input::InputManager: the input source cannot be null");
    }
}

void InputManager::bindKey(const std::string &action, Key key)
{
    this->_actions[action].keys.push_back(key);
}

void InputManager::bindGamepadButton(const std::string &action, unsigned int button)
{
    this->_actions[action].buttons.push_back(button);
}

void InputManager::bindGamepadAxis(const std::string &action, GamepadAxis axis, AxisDirection direction)
{
    this->_actions[action].axes.push_back(AxisBinding{axis, direction});
}

void InputManager::unbind(const std::string &action)
{
    this->_actions.erase(action);
}

void InputManager::update(void)
{
    this->_source->update();
    for (auto &entry : this->_actions) {
        Action &action = entry.second;

        action.wasPressed = action.pressed;
        action.pressed = this->isDown(action);
    }
}

bool InputManager::isActionPressed(const std::string &action) const
{
    return this->getAction(action).pressed;
}

bool InputManager::isActionJustPressed(const std::string &action) const
{
    const Action &found = this->getAction(action);

    return found.pressed && !found.wasPressed;
}

bool InputManager::isActionJustReleased(const std::string &action) const
{
    const Action &found = this->getAction(action);

    return !found.pressed && found.wasPressed;
}

InputManager::Bindings InputManager::getBindings(const std::string &action) const
{
    const Action &found = this->getAction(action);

    return Bindings{found.keys, found.buttons, found.axes};
}

bool InputManager::isDown(const Action &action) const
{
    for (const Key key : action.keys) {
        if (this->_source->isKeyDown(key)) {
            return true;
        }
    }
    for (const unsigned int button : action.buttons) {
        if (this->_source->isGamepadButtonDown(button)) {
            return true;
        }
    }
    for (const AxisBinding &binding : action.axes) {
        const float position = this->_source->getGamepadAxis(binding.axis);

        if (binding.direction == AxisDirection::Positive && position > AXIS_THRESHOLD) {
            return true;
        }
        if (binding.direction == AxisDirection::Negative && position < -AXIS_THRESHOLD) {
            return true;
        }
    }
    return false;
}

const InputManager::Action &InputManager::getAction(const std::string &action) const
{
    static const Action never;
    const auto it = this->_actions.find(action);

    if (it == this->_actions.end()) {
        return never;
    }
    return it->second;
}

} // namespace younity::input
