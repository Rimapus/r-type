/*
** EPITECH PROJECT, 2026
** younity-engine/modules/input/src/SfmlInputSource
** File description:
** SfmlInputSource source file
*/

#include "younity/input/SfmlInputSource.hpp"
#include <SFML/Window/Joystick.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <array>
#include <cmath>
#include <cstddef>

namespace younity::input {

namespace {

// The SFML keys, in the same order as Key
constexpr std::array SFML_KEYS = {
    sf::Keyboard::Key::A,
    sf::Keyboard::Key::B,
    sf::Keyboard::Key::C,
    sf::Keyboard::Key::D,
    sf::Keyboard::Key::E,
    sf::Keyboard::Key::F,
    sf::Keyboard::Key::G,
    sf::Keyboard::Key::H,
    sf::Keyboard::Key::I,
    sf::Keyboard::Key::J,
    sf::Keyboard::Key::K,
    sf::Keyboard::Key::L,
    sf::Keyboard::Key::M,
    sf::Keyboard::Key::N,
    sf::Keyboard::Key::O,
    sf::Keyboard::Key::P,
    sf::Keyboard::Key::Q,
    sf::Keyboard::Key::R,
    sf::Keyboard::Key::S,
    sf::Keyboard::Key::T,
    sf::Keyboard::Key::U,
    sf::Keyboard::Key::V,
    sf::Keyboard::Key::W,
    sf::Keyboard::Key::X,
    sf::Keyboard::Key::Y,
    sf::Keyboard::Key::Z,
    sf::Keyboard::Key::Num0,
    sf::Keyboard::Key::Num1,
    sf::Keyboard::Key::Num2,
    sf::Keyboard::Key::Num3,
    sf::Keyboard::Key::Num4,
    sf::Keyboard::Key::Num5,
    sf::Keyboard::Key::Num6,
    sf::Keyboard::Key::Num7,
    sf::Keyboard::Key::Num8,
    sf::Keyboard::Key::Num9,
    sf::Keyboard::Key::Left,
    sf::Keyboard::Key::Right,
    sf::Keyboard::Key::Up,
    sf::Keyboard::Key::Down,
    sf::Keyboard::Key::Space,
    sf::Keyboard::Key::Enter,
    sf::Keyboard::Key::Escape,
    sf::Keyboard::Key::Backspace,
    sf::Keyboard::Key::Tab,
    sf::Keyboard::Key::LShift,
    sf::Keyboard::Key::RShift,
    sf::Keyboard::Key::LControl,
    sf::Keyboard::Key::RControl,
    sf::Keyboard::Key::LAlt,
    sf::Keyboard::Key::RAlt,
    sf::Keyboard::Key::F1,
    sf::Keyboard::Key::F2,
    sf::Keyboard::Key::F3,
    sf::Keyboard::Key::F4,
    sf::Keyboard::Key::F5,
    sf::Keyboard::Key::F6,
    sf::Keyboard::Key::F7,
    sf::Keyboard::Key::F8,
    sf::Keyboard::Key::F9,
    sf::Keyboard::Key::F10,
    sf::Keyboard::Key::F11,
    sf::Keyboard::Key::F12,
};

// The SFML axes, in the same order as GamepadAxis
constexpr std::array SFML_AXES = {
    sf::Joystick::Axis::X,
    sf::Joystick::Axis::Y,
    sf::Joystick::Axis::Z,
    sf::Joystick::Axis::R,
    sf::Joystick::Axis::U,
    sf::Joystick::Axis::V,
    sf::Joystick::Axis::PovX,
    sf::Joystick::Axis::PovY,
};

// A key or an axis added to an enum must be added to its array too
static_assert(SFML_KEYS.size() == static_cast<std::size_t>(Key::F12) + 1);
static_assert(SFML_AXES.size() == static_cast<std::size_t>(GamepadAxis::PovY) + 1);

// SFML gives the position of an axis between -100 and 100
constexpr float SFML_AXIS_RANGE = 100.0F;

} // namespace

void SfmlInputSource::update(void)
{
    sf::Joystick::update();
}

bool SfmlInputSource::isKeyDown(Key key) const
{
    return sf::Keyboard::isKeyPressed(SFML_KEYS[static_cast<std::size_t>(key)]);
}

bool SfmlInputSource::isGamepadButtonDown(unsigned int button) const
{
    if (button >= sf::Joystick::ButtonCount) {
        return false;
    }
    for (unsigned int gamepad = 0; gamepad < sf::Joystick::Count; ++gamepad) {
        if (sf::Joystick::isConnected(gamepad) && sf::Joystick::isButtonPressed(gamepad, button)) {
            return true;
        }
    }
    return false;
}

float SfmlInputSource::getGamepadAxis(GamepadAxis axis) const
{
    const sf::Joystick::Axis sfmlAxis = SFML_AXES[static_cast<std::size_t>(axis)];
    float position = 0.0F;

    for (unsigned int gamepad = 0; gamepad < sf::Joystick::Count; ++gamepad) {
        if (!sf::Joystick::isConnected(gamepad)) {
            continue;
        }
        const float current = sf::Joystick::getAxisPosition(gamepad, sfmlAxis) / SFML_AXIS_RANGE;

        if (std::abs(current) > std::abs(position)) {
            position = current;
        }
    }
    return position;
}

} // namespace younity::input
