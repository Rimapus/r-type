/*
** EPITECH PROJECT, 2026
** younity-engine/modules/plugin/src/PluginError
** File description:
** PluginError source file
*/

#include "younity/plugin/PluginError.hpp"

namespace younity::plugin {

std::string_view toString(PluginErrorCode code) noexcept
{
    switch (code) {
    case PluginErrorCode::NotFound:
        return "not found";
    case PluginErrorCode::OpenFailed:
        return "cannot open";
    case PluginErrorCode::MissingDescriptor:
        return "missing descriptor";
    case PluginErrorCode::AbiMismatch:
        return "ABI mismatch";
    case PluginErrorCode::InvalidDescriptor:
        return "invalid descriptor";
    case PluginErrorCode::InvalidConfig:
        return "invalid configuration";
    }
    return "unknown";
}

} // namespace younity::plugin
