/*
** EPITECH PROJECT, 2026
** younity-engine/modules/scripting/src/LogLibrary
** File description:
** LogLibrary source file
*/

#include "LogLibrary.hpp"
#include "younity/logger/Level.hpp"
#include "younity/scripting/Log.hpp"

#include <string>

namespace younity::scripting {

namespace {

/**
 * @brief Get a function writing a message of a script at a level.
 *
 * @param level The level of the messages
 *
 * @return The function, taking the message.
 */
auto writerOf(logger::Level level)
{
    return [level](sol::this_state state, const std::string &message) {
        // Level 1 is the script calling the function
        luaL_where(state, 1);
        const std::string where = sol::stack::pop<std::string>(state);

        logger().log(level, "{}{}", where, message);
    };
}

} // namespace

void openLogLibrary(sol::state &lua)
{
    sol::table library = lua.create_named_table("log");

    library.set_function("debug", writerOf(logger::Level::Debug));
    library.set_function("info", writerOf(logger::Level::Info));
    library.set_function("warn", writerOf(logger::Level::Warn));
    library.set_function("error", writerOf(logger::Level::Error));
}

} // namespace younity::scripting
