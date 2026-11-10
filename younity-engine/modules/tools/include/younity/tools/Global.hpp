/*
** EPITECH PROJECT, 2026
** younity-engine/modules/tools/include/younity/tools/Global
** File description:
** Global header file
*/

#pragma once

#include "younity/tools/Console.hpp"
#include "younity/tools/MetricsStore.hpp"

namespace younity::tools {

/**
 * @brief Get the metrics store shared by the whole engine and the game.
 *        It is created at its first use, which is safe from several threads.
 *
 * @return The global store, valid until the end of the program.
 */
[[nodiscard]] MetricsStore &metrics(void);

/**
 * @brief Get the console shared by the whole engine and the game.
 *        It is created at its first use.
 *
 * @return The global console, valid until the end of the program.
 */
[[nodiscard]] Console &console(void);

} // namespace younity::tools
