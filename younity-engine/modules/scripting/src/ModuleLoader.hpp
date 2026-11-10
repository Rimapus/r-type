/*
** EPITECH PROJECT, 2026
** younity-engine/modules/scripting/src/ModuleLoader
** File description:
** ModuleLoader header file, private to the scripting module
*/

#pragma once

#include <filesystem>
#include <sol/sol.hpp>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace younity::scripting {

/**
 * @brief The require function of the scripts: it loads a Lua module from the folder of the scripts once per version
 *        of its file, e.g. require("scripts.rooms") runs scripts/rooms.lua and returns what the file returns.
 *        It must be destroyed before the interpreter, which holds what the modules returned.
 */
class ModuleLoader
{
public:
    /**
     * @brief A module file a script required, and when the file was last written as it was loaded.
     */
    struct Required
    {
        std::string path;
        std::filesystem::file_time_type version;
    };

    /**
     * @brief Add the require function to the interpreter.
     *
     * @param lua The interpreter
     * @param root The folder the names of the modules are relative to
     */
    void open(sol::state &lua, std::filesystem::path root);

    /**
     * @brief Write the modules required from now on in a list, e.g. while a script starts, or stop with null.
     *
     * @param required The list, kept until the next call
     */
    void record(std::vector<Required> *required) noexcept;

private:
    /**
     * @brief What a module returned, and the version of its file.
     */
    struct Loaded
    {
        sol::object value;
        std::filesystem::file_time_type version;
    };

    /**
     * @brief Get a module, loading its file when it was never loaded or was saved since.
     *
     * @param state The interpreter of the calling script
     * @param name The name of the module, e.g. "scripts.rooms"
     *
     * @return What the file returned, true when it returned nothing.
     *
     * @throw std::invalid_argument for a wrong name or a missing file, std::logic_error for a module requiring itself,
     *        std::runtime_error for a module that fails to load or run.
     */
    sol::object require(sol::this_state state, const std::string &name);

    std::filesystem::path _root;
    std::unordered_map<std::string, Loaded> _loaded;
    // The modules being loaded, to stop a module requiring itself
    std::unordered_set<std::string> _loading;
    std::vector<Required> *_required = nullptr;
};

} // namespace younity::scripting
