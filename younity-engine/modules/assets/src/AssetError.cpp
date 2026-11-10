/*
** EPITECH PROJECT, 2026
** younity-engine/modules/assets/src/AssetError
** File description:
** AssetError source file
*/

#include "younity/assets/AssetError.hpp"

namespace younity::assets {

std::string_view toString(AssetError error) noexcept
{
    switch (error) {
    case AssetError::NotFound:
        return "not found";
    case AssetError::LoadFailed:
        return "load failed";
    case AssetError::InvalidPath:
        return "invalid path";
    case AssetError::NoLoader:
        return "no loader";
    }
    return "unknown";
}

} // namespace younity::assets
