/*
** EPITECH PROJECT, 2026
** j-type/common/include/jtype/common/Log
** File description:
** Log header file
*/

#pragma once

#include "younity/logger/Global.hpp"
#include "younity/logger/Logger.hpp"

namespace jtype {

/**
 * @brief Get the logger of the game, named "j-type" in the messages. Its messages go where the ones of the engine go:
 *        the terminal, the console window of the editor and the log file set by YOUNITY_LOG_FILE.
 *
 * @return The logger, valid until the end of the program.
 */
[[nodiscard]] inline younity::logger::Logger &logger(void)
{
    // Looked up once: the next calls do not lock the registry
    static younity::logger::Logger &gameLogger = younity::logger::get("j-type");

    return gameLogger;
}

} // namespace jtype
