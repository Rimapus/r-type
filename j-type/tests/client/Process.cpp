/*
** EPITECH PROJECT, 2026
** j-type/tests/client/Process
** File description:
** Unit tests of the programs the client starts
*/

#include <criterion/criterion.h>

#include <chrono>
#include <thread>

#include "system/Process.hpp"

using jtype::Process;

Test(client_process, does_not_start_a_missing_program)
{
    Process process;

    cr_assert_not(process.start("missing/program", {}));
    cr_assert_not(process.isRunning());
}

Test(client_process, runs_until_the_program_ends)
{
    Process process;

    cr_assert(process.start("/bin/sh", {"-c", "sleep 0.2"}));
    cr_assert(process.isRunning());
    cr_assert_not(process.start("/bin/sh", {"-c", "exit 0"}));
    for (int tries = 0; tries < 100 && process.isRunning(); ++tries) {
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
    cr_assert_not(process.isRunning());
    cr_assert(process.start("/bin/sh", {"-c", "exit 0"}));
}
