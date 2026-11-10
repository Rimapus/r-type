/*
** EPITECH PROJECT, 2026
** younity-engine/modules/core/src/Version
** File description:
** Version source file
*/

#include "younity/core/Version.hpp"

namespace younity {

// Prevent the missing YOUNITY_VERSION from the CMAKE.
#ifndef YOUNITY_VERSION
#error "YOUNITY_VERSION must be defined by the build system"
#endif

std::string_view version(void) noexcept
{
    return YOUNITY_VERSION;
}

} // namespace younity
