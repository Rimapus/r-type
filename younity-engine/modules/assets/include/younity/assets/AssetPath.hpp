/*
** EPITECH PROJECT, 2026
** younity-engine/modules/assets/include/younity/assets/AssetPath
** File description:
** AssetPath header file
*/

#pragma once

#include <expected>
#include <filesystem>

#include "younity/assets/AssetError.hpp"

namespace younity::assets {

/**
 * @brief Check the path of a resource and give its normal form, the same for every way to write it.
 *
 * The path is relative to the asset folder and has to stay inside it,
 * e.g. "sprites/../ship.png" gives "ship.png" and "../save.dat" is refused.
 *
 * @param path The path of the resource, relative to the asset folder
 *
 * @return The normal form of the path, or AssetError::InvalidPath if it is empty,
 *         absolute, a folder or goes out of the asset folder.
 */
[[nodiscard]] std::expected<std::filesystem::path, AssetError> normalizeAssetPath(const std::filesystem::path &path);

} // namespace younity::assets
