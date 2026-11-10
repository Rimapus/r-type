/*
** EPITECH PROJECT, 2026
** younity-engine/modules/core/include/younity/core/Version
** File description:
** Version header file
*/

#pragma once

#include <string_view>

namespace younity {

/**
 * @brief Get the current version of the Younity Engine.
 *
 * @return The engine version as "MAJOR.MINOR.PATCH", e.g. "0.9.0".
 */
[[nodiscard]] std::string_view version(void) noexcept;

} // namespace younity
