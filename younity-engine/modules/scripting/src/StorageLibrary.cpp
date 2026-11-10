/*
** EPITECH PROJECT, 2026
** younity-engine/modules/scripting/src/StorageLibrary
** File description:
** StorageLibrary source file
*/

#include "StorageLibrary.hpp"
#include "younity/scripting/Log.hpp"

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <limits>
#include <nlohmann/json.hpp>
#include <stdexcept>
#include <string>
#include <system_error>
#include <utility>

namespace younity::scripting {

namespace {

// Past this depth, a table is taken for one that contains itself
constexpr int MAX_DEPTH = 32;
constexpr std::size_t MAX_NAME_LENGTH = 64;

/**
 * @brief Get the file of a save from its name.
 *
 * @param saves The folder of the saves
 * @param name The name given by the script
 *
 * @return The JSON file of the save.
 *
 * @throw std::invalid_argument if the name is not made of letters, digits, '_' and '-', or is too long.
 */
std::filesystem::path fileOf(const std::filesystem::path &saves, const std::string &name)
{
    const bool valid = !name.empty() && name.size() <= MAX_NAME_LENGTH &&
                       name.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_-") ==
                           std::string::npos;

    if (!valid) {
        throw std::invalid_argument("the save '" + name + "' must be named with up to 64 letters, digits, '_' and '-'");
    }
    return saves / (name + ".json");
}

/**
 * @brief Tell if a Lua number is an integer, e.g. 3 but not 3.0.
 */
bool isInteger(const sol::object &number)
{
    lua_State *state = number.lua_state();

    number.push(state);
    const bool integer = lua_isinteger(state, -1) != 0;

    lua_pop(state, 1);
    return integer;
}

/**
 * @brief Convert a value of a script to JSON.
 *
 * @param value The value
 * @param depth How many tables contain it
 *
 * @return The JSON value.
 *
 * @throw std::invalid_argument for a value JSON cannot hold: a function, a non-finite number, a table mixing keys or
 *        too deep.
 */
nlohmann::json toJson(const sol::object &value, int depth)
{
    switch (value.get_type()) {
    case sol::type::boolean:
        return value.as<bool>();
    case sol::type::string:
        return value.as<std::string>();
    case sol::type::number: {
        if (isInteger(value)) {
            return value.as<std::int64_t>();
        }
        const double number = value.as<double>();

        if (!std::isfinite(number)) {
            throw std::invalid_argument("a saved number must be finite");
        }
        return number;
    }
    case sol::type::table:
        break;
    default:
        throw std::invalid_argument("a save holds booleans, numbers, strings and tables only");
    }
    if (depth >= MAX_DEPTH) {
        throw std::invalid_argument("a saved table holds at most 32 levels of tables, and never itself");
    }
    const sol::table table = value.as<sol::table>();
    const std::size_t length = table.size();
    std::size_t count = 0;

    for (const auto &entry : table) {
        (void)entry;
        ++count;
    }
    // Only the keys 1 to n: an array, so the order is kept
    if (length > 0 && count == length) {
        nlohmann::json array = nlohmann::json::array();

        for (std::size_t index = 1; index <= length; ++index) {
            array.push_back(toJson(table.raw_get<sol::object>(index), depth + 1));
        }
        return array;
    }
    nlohmann::json object = nlohmann::json::object();

    for (const auto &[key, field] : table) {
        if (key.get_type() != sol::type::string) {
            throw std::invalid_argument("a saved table has either the keys 1 to n, or names");
        }
        object[key.as<std::string>()] = toJson(field, depth + 1);
    }
    return object;
}

/**
 * @brief Convert a JSON value read from a save to a value of the scripts.
 *
 * @param lua The interpreter
 * @param json The value
 *
 * @return The Lua value, nil for null.
 */
sol::object toLua(sol::state_view lua, const nlohmann::json &json)
{
    switch (json.type()) {
    case nlohmann::json::value_t::boolean:
        return sol::make_object(lua, json.get<bool>());
    case nlohmann::json::value_t::string:
        return sol::make_object(lua, json.get<std::string>());
    case nlohmann::json::value_t::number_integer:
        return sol::make_object(lua, json.get<std::int64_t>());
    case nlohmann::json::value_t::number_unsigned:
        // The parser reads every integer from 0 as unsigned: one too large for a Lua integer stays a number
        if (json.get<std::uint64_t>() <= static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max())) {
            return sol::make_object(lua, json.get<std::int64_t>());
        }
        return sol::make_object(lua, json.get<double>());
    case nlohmann::json::value_t::number_float:
        return sol::make_object(lua, json.get<double>());
    case nlohmann::json::value_t::array: {
        sol::table table = lua.create_table();

        for (std::size_t index = 0; index < json.size(); ++index) {
            table[index + 1] = toLua(lua, json[index]);
        }
        return table;
    }
    case nlohmann::json::value_t::object: {
        sol::table table = lua.create_table();

        for (const auto &[key, field] : json.items()) {
            table[key] = toLua(lua, field);
        }
        return table;
    }
    default:
        return sol::lua_nil;
    }
}

/**
 * @brief Write a save next to its file, then put it in its place: a crash while writing keeps the previous save.
 *
 * @return true if the save is in its file.
 */
bool write(const std::filesystem::path &file, const nlohmann::json &json)
{
    std::error_code error;

    std::filesystem::create_directories(file.parent_path(), error);
    if (error) {
        logger().warn("cannot create the folder of the saves '{}': {}", file.parent_path().string(), error.message());
        return false;
    }
    std::filesystem::path temporary = file;

    temporary += ".tmp";
    {
        std::ofstream output(temporary, std::ios::binary | std::ios::trunc);

        output << json.dump(4) << '\n';
        if (!output) {
            logger().warn("cannot write the save '{}'", temporary.string());
            return false;
        }
    }
    std::filesystem::rename(temporary, file, error);
    if (error) {
        logger().warn("cannot write the save '{}': {}", file.string(), error.message());
        return false;
    }
    return true;
}

} // namespace

void openStorageLibrary(sol::state &lua, std::filesystem::path saves)
{
    sol::table library = lua.create_named_table("storage");

    library.set_function("save", [saves](const std::string &name, const sol::table &table) {
        const std::filesystem::path file = fileOf(saves, name);
        // Converted first: a table JSON cannot hold raises an error even when the game gives no folder
        const nlohmann::json json = toJson(table, 0);

        return !saves.empty() && write(file, json);
    });
    library.set_function("load", [saves = std::move(saves)](sol::this_state state, const std::string &name) {
        const std::filesystem::path file = fileOf(saves, name);

        if (saves.empty()) {
            return sol::make_object(state, sol::lua_nil);
        }
        std::ifstream input(file, std::ios::binary);

        if (!input) {
            return sol::make_object(state, sol::lua_nil);
        }
        const nlohmann::json json = nlohmann::json::parse(input, nullptr, false);

        if (json.is_discarded()) {
            logger().warn("the save '{}' is not valid JSON", file.string());
            return sol::make_object(state, sol::lua_nil);
        }
        return toLua(state, json);
    });
}

} // namespace younity::scripting
