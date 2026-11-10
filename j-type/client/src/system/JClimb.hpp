/*
** EPITECH PROJECT, 2026
** j-type/client/src/system/JClimb
** File description:
** JClimb header file
*/

#pragma once

#include <filesystem>

#include "system/Process.hpp"

namespace jtype {

/**
 * @brief Find the folder of J-Climb: next to the client, where cmake/JClimb.cmake builds it and the packages keep it,
 *        whatever folder the client runs from.
 *
 * @return The folder that holds the program and its assets folder.
 *
 * @throw std::runtime_error if the system cannot give the path of the client.
 */
[[nodiscard]] std::filesystem::path getJClimbFolder(void);

/**
 * @brief Start J-Climb, given the path of its assets folder, unless it still runs. A missing J-Climb is only logged.
 *
 * @param jclimb The process of J-Climb, kept by the client
 */
void startJClimb(Process &jclimb);

} // namespace jtype
