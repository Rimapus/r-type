/*
** EPITECH PROJECT, 2026
** younity-engine/modules/input/src/Names
** File description:
** Names source file
*/

#include "younity/input/Names.hpp"

#include <array>
#include <cstddef>
#include <string>
#include <utility>

namespace younity::input {

namespace {

// In the order of the Key enum, so a key is the index of its name
constexpr std::array<std::string_view, 63> KEY_NAMES{
    "A",
    "B",
    "C",
    "D",
    "E",
    "F",
    "G",
    "H",
    "I",
    "J",
    "K",
    "L",
    "M",
    "N",
    "O",
    "P",
    "Q",
    "R",
    "S",
    "T",
    "U",
    "V",
    "W",
    "X",
    "Y",
    "Z",
    "Num0",
    "Num1",
    "Num2",
    "Num3",
    "Num4",
    "Num5",
    "Num6",
    "Num7",
    "Num8",
    "Num9",
    "Left",
    "Right",
    "Up",
    "Down",
    "Space",
    "Enter",
    "Escape",
    "Backspace",
    "Tab",
    "LShift",
    "RShift",
    "LControl",
    "RControl",
    "LAlt",
    "RAlt",
    "F1",
    "F2",
    "F3",
    "F4",
    "F5",
    "F6",
    "F7",
    "F8",
    "F9",
    "F10",
    "F11",
    "F12",
};

static_assert(KEY_NAMES.size() == static_cast<std::size_t>(Key::F12) + 1, "every key has a name");

// In the order of the GamepadAxis enum
constexpr std::array<std::string_view, 8> AXIS_NAMES{"X", "Y", "Z", "R", "U", "V", "PovX", "PovY"};

static_assert(AXIS_NAMES.size() == static_cast<std::size_t>(GamepadAxis::PovY) + 1, "every axis has a name");

/**
 * @brief Find the index of a name in a list of names.
 *
 * @param names The names, in the order of their enum
 * @param name The name to find
 *
 * @return The index, or nothing if the name is not in the list.
 */
template <std::size_t Size>
std::optional<std::size_t> indexOf(const std::array<std::string_view, Size> &names, std::string_view name) noexcept
{
    for (std::size_t index = 0; index < Size; ++index) {
        if (names[index] == name) {
            return index;
        }
    }
    return std::nullopt;
}

} // namespace

std::string_view toString(Key key) noexcept
{
    return KEY_NAMES[static_cast<std::size_t>(key)];
}

std::optional<Key> parseKey(std::string_view name) noexcept
{
    const std::optional<std::size_t> index = indexOf(KEY_NAMES, name);

    return index ? std::optional(static_cast<Key>(*index)) : std::nullopt;
}

std::string_view toString(GamepadAxis axis) noexcept
{
    return AXIS_NAMES[static_cast<std::size_t>(axis)];
}

std::optional<GamepadAxis> parseGamepadAxis(std::string_view name) noexcept
{
    const std::optional<std::size_t> index = indexOf(AXIS_NAMES, name);

    return index ? std::optional(static_cast<GamepadAxis>(*index)) : std::nullopt;
}

std::vector<std::string> describe(const InputManager::Bindings &bindings)
{
    std::vector<std::string> names;

    for (const Key key : bindings.keys) {
        names.emplace_back(toString(key));
    }
    for (const unsigned int button : bindings.buttons) {
        names.push_back("Button " + std::to_string(button));
    }
    for (const InputManager::AxisBinding &binding : bindings.axes) {
        names.push_back(std::string(toString(binding.axis)) + (binding.direction == AxisDirection::Positive ? "+" : "-"));
    }
    return names;
}

} // namespace younity::input
