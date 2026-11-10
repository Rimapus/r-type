/*
** EPITECH PROJECT, 2026
** j-type/tests/common/Paths
** File description:
** Paths unit tests
*/

#include <criterion/criterion.h>

#include <filesystem>

#include "jtype/common/Paths.hpp"

Test(executable_dir, is_an_absolute_folder)
{
    const std::filesystem::path folder = jtype::executableDir();

    cr_assert(folder.is_absolute());
    cr_assert(std::filesystem::is_directory(folder));
}

Test(executable_dir, contains_the_tests_executable)
{
#if defined(_WIN32)
    cr_assert(std::filesystem::exists(jtype::executableDir() / "j-type_tests.exe"));
#else
    cr_assert(std::filesystem::exists(jtype::executableDir() / "j-type_tests"));
#endif
}

Test(executable_dir, does_not_depend_on_the_working_folder)
{
    const std::filesystem::path before = jtype::executableDir();
    const std::filesystem::path working = std::filesystem::current_path();

    std::filesystem::current_path(std::filesystem::temp_directory_path());
    const std::filesystem::path after = jtype::executableDir();

    std::filesystem::current_path(working);
    cr_assert(before == after);
}
