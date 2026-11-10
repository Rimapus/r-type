/*
** EPITECH PROJECT, 2026
** younity-engine/modules/scripting/src/InputLibrary
** File description:
** InputLibrary source file
*/

#include "InputLibrary.hpp"

#include <cmath>
#include <functional>
#include <limits>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace younity::scripting {

namespace {

/**
 * @brief Get the Lua function asking the game about an action.
 *
 * @param question The function of the game, false for every action if it is empty
 *
 * @return The function, taking the name of the action.
 */
auto askerOf(std::function<bool(const std::string &)> question)
{
    return [question = std::move(question)](const std::string &action) { return question && question(action); };
}

/**
 * @brief Read the number of a gamepad button given by a script.
 *
 * @param button The number given by the script
 *
 * @return The button.
 *
 * @throw std::invalid_argument if the number is not an integer from 0.
 */
unsigned int toButton(double button)
{
    if (std::floor(button) != button || button < 0 || button > std::numeric_limits<unsigned int>::max()) {
        throw std::invalid_argument("the button must be an integer from 0");
    }
    return static_cast<unsigned int>(button);
}

} // namespace

void openInputLibrary(sol::state &lua, const Actions &actions)
{
    sol::table library = lua.create_named_table("input");

    library.set_function("isPressed", askerOf(actions.isPressed));
    library.set_function("isJustPressed", askerOf(actions.isJustPressed));
    library.set_function("isJustReleased", askerOf(actions.isJustReleased));
    library.set_function("bindKey", [bind = actions.bindKey](const std::string &action, const std::string &key) {
        if (bind && !bind(action, key)) {
            throw std::invalid_argument("there is no key named '" + key + "'");
        }
    });
    library.set_function("bindButton", [bind = actions.bindButton](const std::string &action, double button) {
        const unsigned int number = toButton(button);

        if (bind) {
            bind(action, number);
        }
    });
    library.set_function("bindAxis", [bind = actions.bindAxis](const std::string &action, const std::string &axis,
                                                               double direction) {
        if (direction != 1 && direction != -1) {
            throw std::invalid_argument("the direction of the axis must be 1 or -1");
        }
        if (bind && !bind(action, axis, direction > 0)) {
            throw std::invalid_argument("there is no gamepad axis named '" + axis + "'");
        }
    });
    library.set_function("unbind", [unbind = actions.unbind](const std::string &action) {
        if (unbind) {
            unbind(action);
        }
    });
    library.set_function("getBindings", [get = actions.getBindings](const std::string &action) {
        return sol::as_table(get ? get(action) : std::vector<std::string>{});
    });
}

} // namespace younity::scripting
