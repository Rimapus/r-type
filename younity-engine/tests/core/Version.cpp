/*
** EPITECH PROJECT, 2026
** younity-engine/tests/core/Version
** File description:
** Version unit tests
*/

#include <criterion/criterion.h>

#include <regex>
#include <string>

#include "younity/core/Version.hpp"

Test(core_version, is_not_empty)
{
    cr_assert(!younity::version().empty());
}

Test(core_version, follows_major_minor_patch_format)
{
    const std::string version(younity::version());

    cr_assert(std::regex_match(version, std::regex(R"(\d+\.\d+\.\d+)")), "got \"%s\"", version.c_str());
}
