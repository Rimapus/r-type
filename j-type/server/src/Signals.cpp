/*
** EPITECH PROJECT, 2026
** j-type/server/src/Signals
** File description:
** Signals source file
*/

#include "Signals.hpp"

#include <csignal>

namespace jtype {

namespace {

// Written by the signal handler and read by the loop, so it has the only type the standard allows for that
volatile std::sig_atomic_t g_stopRequested = 0;

/**
 * @brief Remember that the server was asked to stop. A signal handler may only do such a store.
 *
 * @param signal The signal received, unused
 */
void onStopSignal(int signal)
{
    (void)signal;
    g_stopRequested = 1;
}

} // namespace

void installStopHandlers(void) noexcept
{
    (void)std::signal(SIGINT, onStopSignal);
    (void)std::signal(SIGTERM, onStopSignal);
}

bool stopRequested(void) noexcept
{
    return g_stopRequested != 0;
}

} // namespace jtype
