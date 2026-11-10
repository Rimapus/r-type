/*
** EPITECH PROJECT, 2026
** younity-engine/modules/assets/include/younity/assets/AssetError
** File description:
** AssetError header file
*/

#pragma once

#include <string_view>

namespace younity::assets {

/**
 * @brief Why a resource could not be loaded.
 */
enum class AssetError {
    NotFound,
    LoadFailed,
    InvalidPath,
    NoLoader
};

/**
 * @brief Get a short human-readable description of an asset error.
 *
 * @param error The error to describe
 *
 * @return The description, e.g. "not found".
 */
[[nodiscard]] std::string_view toString(AssetError error) noexcept;

} // namespace younity::assets
