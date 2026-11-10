/*
** EPITECH PROJECT, 2026
** j-type/tests/client/JClimb
** File description:
** JClimb unit tests
*/

#include <criterion/criterion.h>

#include <filesystem>

#include "jtype/common/Paths.hpp"
#include "system/JClimb.hpp"
#include "system/Process.hpp"

using jtype::Process;

Test(client_jclimb, is_next_to_the_client_whatever_the_working_folder)
{
    const std::filesystem::path folder = jtype::getJClimbFolder();

    cr_assert(folder.is_absolute());
    cr_assert(folder == jtype::executableDir() / "j-climb");
}

Test(client_jclimb, is_not_started_when_it_is_missing)
{
    // No J-Climb is built next to the tests
    Process jclimb;

    jtype::startJClimb(jclimb);
    cr_assert_not(jclimb.isRunning());
}
