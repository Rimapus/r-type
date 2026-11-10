/*
** EPITECH PROJECT, 2026
** j-type/tests/server/Signals
** File description:
** Signals unit tests
*/

#include <criterion/criterion.h>

#include <csignal>

#include "Signals.hpp"

Test(stop_signals, a_sigint_asks_the_server_to_stop)
{
    jtype::installStopHandlers();
    cr_assert_not(jtype::stopRequested());
    cr_assert_eq(std::raise(SIGINT), 0);
    cr_assert(jtype::stopRequested());
}

Test(stop_signals, a_sigterm_asks_the_server_to_stop)
{
    jtype::installStopHandlers();
    cr_assert_eq(std::raise(SIGTERM), 0);
    cr_assert(jtype::stopRequested());
}
