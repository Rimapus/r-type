/*
** EPITECH PROJECT, 2026
** j-type/common/include/jtype/common/Paths
** File description:
** Paths header file
*/

#pragma once

#include <filesystem>

namespace jtype {

/**
 * @brief Find the folder of the running executable, so the files next to it are found from any working folder.
 *
 * @return The absolute path of the folder.
 *
 * @throw std::runtime_error if the system cannot give the path of the executable.
 */
[[nodiscard]] std::filesystem::path executableDir(void);

} // namespace jtype
