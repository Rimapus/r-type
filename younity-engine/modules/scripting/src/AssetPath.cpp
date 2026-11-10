/*
** EPITECH PROJECT, 2026
** younity-engine/modules/scripting/src/AssetPath
** File description:
** AssetPath source file
*/

#include "AssetPath.hpp"

#include <filesystem>
#include <stdexcept>

namespace younity::scripting {

void checkAssetPath(const std::string &path, const std::string &what)
{
    const std::filesystem::path file(path);

    if (path.empty() || file.has_root_path()) {
        throw std::invalid_argument(what + " '" + path + "' must be a path relative to the assets");
    }
    for (const std::filesystem::path &part : file) {
        if (part == "..") {
            throw std::invalid_argument(what + " '" + path + "' must stay inside the assets");
        }
    }
}

} // namespace younity::scripting
