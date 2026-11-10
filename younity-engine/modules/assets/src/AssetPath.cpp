/*
** EPITECH PROJECT, 2026
** younity-engine/modules/assets/src/AssetPath
** File description:
** AssetPath source file
*/

#include "younity/assets/AssetPath.hpp"

namespace younity::assets {

std::expected<std::filesystem::path, AssetError> normalizeAssetPath(const std::filesystem::path &path)
{
    // has_root_path also catches "/file" on Windows, which is not absolute there but leaves the folder
    if (path.empty() || path.has_root_path()) {
        return std::unexpected(AssetError::InvalidPath);
    }

    const std::filesystem::path normal = path.lexically_normal();

    // The normal form keeps the ".." it cannot resolve at its start, and ends with a "/" for a folder
    if (normal == "." || *normal.begin() == ".." || !normal.has_filename()) {
        return std::unexpected(AssetError::InvalidPath);
    }
    return normal;
}

} // namespace younity::assets
