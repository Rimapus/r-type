/*
** EPITECH PROJECT, 2026
** younity-engine/modules/scripting/src/ModuleLoader
** File description:
** ModuleLoader source file
*/

#include "ModuleLoader.hpp"

#include <algorithm>
#include <optional>
#include <stdexcept>
#include <string>
#include <system_error>
#include <utility>

namespace younity::scripting {

namespace {

/**
 * @brief Get the file of a module from its name.
 *
 * @param name The name given by the script, e.g. "scripts.rooms"
 *
 * @return The path of the file, relative to the folder of the scripts, e.g. "scripts/rooms.lua".
 *
 * @throw std::invalid_argument unless the name is made of words of letters, digits and '_' separated by dots.
 */
std::string fileOf(const std::string &name)
{
    const auto isAllowed = [](char character) {
        return (character >= 'a' && character <= 'z') || (character >= 'A' && character <= 'Z') ||
               (character >= '0' && character <= '9') || character == '_' || character == '.';
    };
    // No empty word: no dot at an end, and never two in a row
    const bool valid = !name.empty() && name.front() != '.' && name.back() != '.' &&
                       name.find("..") == std::string::npos && std::ranges::all_of(name, isAllowed);

    if (!valid) {
        throw std::invalid_argument("the module '" + name + "' must be named like 'folder.file'");
    }
    std::string path = name;

    std::ranges::replace(path, '.', '/');
    return path + ".lua";
}

/**
 * @brief Get the message of a failed load or call.
 */
template <typename Result>
std::string errorOf(const Result &result)
{
    const sol::error error = result;

    return error.what();
}

} // namespace

void ModuleLoader::open(sol::state &lua, std::filesystem::path root)
{
    this->_root = std::move(root);
    lua.set_function("require", [this](sol::this_state state, const std::string &name) {
        return this->require(state, name);
    });
}

void ModuleLoader::record(std::vector<Required> *required) noexcept
{
    this->_required = required;
}

sol::object ModuleLoader::require(sol::this_state state, const std::string &name)
{
    const std::string path = fileOf(name);
    std::error_code error;
    const std::filesystem::file_time_type version = std::filesystem::last_write_time(this->_root / path, error);

    if (error) {
        throw std::invalid_argument("there is no module '" + name + "', looked for " + path);
    }
    if (this->_required != nullptr) {
        this->_required->push_back(Required{path, version});
    }
    const auto found = this->_loaded.find(path);

    if (found != this->_loaded.end() && found->second.version == version) {
        return found->second.value;
    }
    if (!this->_loading.insert(path).second) {
        throw std::logic_error("the module '" + name + "' requires itself");
    }
    sol::state_view lua(state);
    const sol::load_result chunk = lua.load_file((this->_root / path).string());
    std::optional<sol::protected_function_result> returned;

    if (chunk.valid()) {
        returned = chunk.get<sol::protected_function>()();
    }
    this->_loading.erase(path);
    if (!chunk.valid()) {
        throw std::runtime_error(errorOf(chunk));
    }
    if (!returned->valid()) {
        throw std::runtime_error(errorOf(*returned));
    }
    // Like the require of Lua, a module returning nothing gives true
    const sol::object value = returned->get_type() == sol::type::none || returned->get_type() == sol::type::lua_nil
                                  ? sol::make_object(lua, true)
                                  : returned->get<sol::object>();

    this->_loaded.insert_or_assign(path, Loaded{value, version});
    return value;
}

} // namespace younity::scripting
