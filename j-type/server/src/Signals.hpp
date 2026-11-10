/*
** EPITECH PROJECT, 2026
** j-type/server/src/Signals
** File description:
** Signals header file
*/

#pragma once

namespace jtype {

/**
 * @brief End the loop of the server on Ctrl+C or on a termination request, instead of killing the process: the socket
 *        and the network threads are then closed cleanly.
 */
void installStopHandlers(void) noexcept;

/**
 * @brief Check whether SIGINT or SIGTERM was received since the handlers were installed.
 *
 * @return true once one of them was received.
 */
[[nodiscard]] bool stopRequested(void) noexcept;

} // namespace jtype
