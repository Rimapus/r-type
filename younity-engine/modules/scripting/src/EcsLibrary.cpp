/*
** EPITECH PROJECT, 2026
** younity-engine/modules/scripting/src/EcsLibrary
** File description:
** EcsLibrary source file
*/

#include "EcsLibrary.hpp"
#include "younity/ecs/ComponentInfo.hpp"
#include "younity/ecs/FieldValue.hpp"

#include <cmath>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

namespace younity::scripting {

namespace {

/**
 * @brief Read an integer given by a script, if it is one.
 *
 * @tparam Integer The type of the integer
 * @param value The value given by the script
 *
 * @return The integer, or an empty optional if the value is not an integer in the range of the type.
 */
template <std::integral Integer>
std::optional<Integer> asInteger(const sol::object &value)
{
    if (value.get_type() != sol::type::number) {
        return std::nullopt;
    }
    const double number = value.as<double>();

    // The bounds are exact in a double: the ones of the 32-bit integers the engine reads
    if (std::trunc(number) != number || number < static_cast<double>(std::numeric_limits<Integer>::min()) ||
        number > static_cast<double>(std::numeric_limits<Integer>::max())) {
        return std::nullopt;
    }
    return static_cast<Integer>(number);
}

/**
 * @brief Read an integer given by a script.
 *
 * @tparam Integer The type of the integer
 * @param value The value given by the script
 * @param what What the value is, for the error message
 *
 * @return The integer.
 *
 * @throw std::invalid_argument if the value is not an integer, or out of the range of the type.
 */
template <std::integral Integer>
Integer toInteger(const sol::object &value, const std::string &what)
{
    const std::optional<Integer> integer = asInteger<Integer>(value);

    if (!integer) {
        throw std::invalid_argument(what + " must be an integer between " +
                                    std::to_string(std::numeric_limits<Integer>::min()) + " and " +
                                    std::to_string(std::numeric_limits<Integer>::max()));
    }
    return *integer;
}

/**
 * @brief Read a number given by a script.
 *
 * @param value The value given by the script
 * @param what What the value is, for the error message
 *
 * @return The number.
 *
 * @throw std::invalid_argument if the value is not a number, or does not fit in a float.
 */
float toFloat(const sol::object &value, const std::string &what)
{
    // NaN and the infinities would spread to the positions, the physics and the drawing
    if (value.get_type() != sol::type::number || !std::isfinite(value.as<double>()) ||
        std::abs(value.as<double>()) > static_cast<double>(std::numeric_limits<float>::max())) {
        throw std::invalid_argument(what + " must be a finite number");
    }
    return value.as<float>();
}

/**
 * @brief Read a table given by a script.
 *
 * @param value The value given by the script
 * @param what What the value is, for the error message
 *
 * @return The table.
 *
 * @throw std::invalid_argument if the value is not a table.
 */
sol::table toTable(const sol::object &value, const std::string &what)
{
    if (value.get_type() != sol::type::table) {
        throw std::invalid_argument(what + " must be a table");
    }
    return value.as<sol::table>();
}

/**
 * @brief Convert the value of a field to Lua: a Vector2f is {x, y}, a Color is {r, g, b, a},
 *        a rectangle is {left, top, width, height} or nil when there is none,
 *        a list of rectangles is an array of them.
 *
 * @param lua The interpreter
 * @param value The value of the field
 *
 * @return The Lua value.
 */
sol::object toLua(sol::state_view lua, const ecs::FieldValue &value)
{
    return std::visit(
        [&lua](const auto &field) -> sol::object {
            using Field = std::decay_t<decltype(field)>;

            if constexpr (std::same_as<Field, Vector2f>) {
                return lua.create_table_with("x", field.x, "y", field.y);
            } else if constexpr (std::same_as<Field, Color>) {
                return lua.create_table_with("r", field.r, "g", field.g, "b", field.b, "a", field.a);
            } else if constexpr (std::same_as<Field, std::optional<Recti>>) {
                if (!field) {
                    return sol::make_object(lua, sol::lua_nil);
                }
                return lua.create_table_with("left", field->left, "top", field->top, "width", field->width, "height",
                                             field->height);
            } else if constexpr (std::same_as<Field, std::vector<Recti>>) {
                sol::table frames = lua.create_table();

                for (std::size_t index = 0; index < field.size(); ++index) {
                    const Recti &frame = field[index];

                    frames[index + 1] = lua.create_table_with("left", frame.left, "top", frame.top, "width", frame.width,
                                                              "height", frame.height);
                }
                return frames;
            } else {
                return sol::make_object(lua, field);
            }
        },
        value);
}

/**
 * @brief Convert a Lua value to the value of a field, the opposite of toLua().
 *        A channel missing from a color is 255, like in Color.
 *
 * @param value The value given by the script
 * @param type The type of the field
 * @param what What the value is, for the error message
 *
 * @return The value of the field.
 *
 * @throw std::invalid_argument if the value does not fit the type of the field.
 */
ecs::FieldValue fromLua(const sol::object &value, ecs::FieldType type, const std::string &what)
{
    switch (type) {
    case ecs::FieldType::Bool:
        if (value.get_type() != sol::type::boolean) {
            throw std::invalid_argument(what + " must be a boolean");
        }
        return value.as<bool>();
    case ecs::FieldType::Int:
        return toInteger<int>(value, what);
    case ecs::FieldType::Float:
        return toFloat(value, what);
    case ecs::FieldType::String:
        if (value.get_type() != sol::type::string) {
            throw std::invalid_argument(what + " must be a string");
        }
        return value.as<std::string>();
    case ecs::FieldType::Vector2f: {
        const sol::table table = toTable(value, what);

        return Vector2f{toFloat(table.raw_get<sol::object>("x"), what + ".x"),
                        toFloat(table.raw_get<sol::object>("y"), what + ".y")};
    }
    case ecs::FieldType::Color: {
        const sol::table table = toTable(value, what);
        const auto channel = [&table, &what](const char *name) {
            const sol::object channelValue = table.raw_get<sol::object>(name);

            return channelValue.get_type() == sol::type::lua_nil ? std::uint8_t{255}
                                                                 : toInteger<std::uint8_t>(channelValue, what + "." + name);
        };

        return Color{channel("r"), channel("g"), channel("b"), channel("a")};
    }
    case ecs::FieldType::OptionalRecti: {
        // A nil field is absent from the table given by the script, so this value is never nil
        const sol::table table = toTable(value, what);

        return std::optional<Recti>{
            Recti{toInteger<int>(table.raw_get<sol::object>("left"), what + ".left"),
                  toInteger<int>(table.raw_get<sol::object>("top"), what + ".top"),
                  toInteger<int>(table.raw_get<sol::object>("width"), what + ".width"),
                  toInteger<int>(table.raw_get<sol::object>("height"), what + ".height")}
        };
    }
    case ecs::FieldType::RectiVector: {
        const sol::table table = toTable(value, what);
        std::vector<Recti> frames;

        frames.reserve(table.size());
        for (std::size_t index = 1; index <= table.size(); ++index) {
            const std::string framePath = what + "[" + std::to_string(index) + "]";
            const sol::table frame = toTable(table.raw_get<sol::object>(index), framePath);

            frames.push_back(
                Recti{toInteger<int>(frame.raw_get<sol::object>("left"), framePath + ".left"),
                      toInteger<int>(frame.raw_get<sol::object>("top"), framePath + ".top"),
                      toInteger<int>(frame.raw_get<sol::object>("width"), framePath + ".width"),
                      toInteger<int>(frame.raw_get<sol::object>("height"), framePath + ".height")});
        }
        return frames;
    }
    }
    throw std::invalid_argument(what + " has an unknown type");
}

/**
 * @brief Get the registry of the step running.
 *
 * @param context The context of the ecs table
 *
 * @return The registry.
 *
 * @throw std::logic_error outside a step.
 */
ecs::Registry &registryOf(const EcsContext &context)
{
    if (context.registry == nullptr) {
        throw std::logic_error("the ecs functions only work during a step");
    }
    return *context.registry;
}

/**
 * @brief Read the id of an entity given by a script.
 *
 * @param registry The registry of the entity
 * @param id The id given by the script
 *
 * @return The entity.
 *
 * @throw std::invalid_argument if the id is not an integer, or the entity is not alive.
 */
ecs::Entity toEntity(const ecs::Registry &registry, const sol::object &id)
{
    const ecs::Entity entity(toInteger<ecs::Entity::Id>(id, "the entity"));

    if (!registry.isAlive(entity)) {
        throw std::invalid_argument("the entity " + std::to_string(entity.getId()) + " is not alive");
    }
    return entity;
}

/**
 * @brief Get a component type that the entity has.
 *
 * @param context The context of the ecs table
 * @param entity The entity
 * @param name The name of the component
 *
 * @return The information of the component type.
 *
 * @throw std::out_of_range if no component type has this name, or the entity does not have it.
 */
const ecs::ComponentInfo &componentOf(const EcsContext &context, ecs::Entity entity, const std::string &name)
{
    const ecs::ComponentInfo &info = context.components.get(name);

    if (!info.has(registryOf(context), entity)) {
        throw std::out_of_range("the entity " + std::to_string(entity.getId()) + " has no " + name);
    }
    return info;
}

/**
 * @brief Add the position of the error to its message, like the errors raised by Lua.
 *        Called by sol2 when a function of the ecs table throws.
 */
int locateError(lua_State *lua, sol::optional<const std::exception &> /* error */, sol::string_view description)
{
    // Level 1 is the script calling the function
    luaL_where(lua, 1);
    std::string message = sol::stack::pop<std::string>(lua);

    message.append(description.data(), description.size());
    return sol::stack::push(lua, message);
}

} // namespace

void openEcsLibrary(sol::state &lua, EcsContext &context)
{
    sol::table library = lua.create_named_table("ecs");

    lua.set_exception_handler(&locateError);
    library.set_function("create", [&context] { return registryOf(context).createEntity().getId(); });
    library.set_function("destroy", [&context](const sol::object &id) {
        ecs::Registry &registry = registryOf(context);
        const ecs::Entity entity = toEntity(registry, id);

        registry.destroyEntity(entity);
        context.destroyed.insert(entity.getId());
    });
    library.set_function("isAlive", [&context](const sol::object &id) {
        const ecs::Registry &registry = registryOf(context);
        const std::optional<ecs::Entity::Id> entity = asInteger<ecs::Entity::Id>(id);

        // A script keeps nil or -1 for no entity: it is not alive either
        return entity && registry.isAlive(ecs::Entity(*entity));
    });
    library.set_function("has", [&context](const sol::object &id, const std::string &name) {
        const ecs::Registry &registry = registryOf(context);

        return context.components.get(name).has(registry, toEntity(registry, id));
    });
    library.set_function("add", [&context](const sol::object &id, const std::string &name) {
        ecs::Registry &registry = registryOf(context);

        context.components.get(name).add(registry, toEntity(registry, id));
    });
    library.set_function("remove", [&context](const sol::object &id, const std::string &name) {
        ecs::Registry &registry = registryOf(context);

        context.components.get(name).remove(registry, toEntity(registry, id));
    });
    library.set_function("get", [&context](sol::this_state state, const sol::object &id, const std::string &name) {
        const ecs::Registry &registry = registryOf(context);
        const ecs::Entity entity = toEntity(registry, id);
        const ecs::ComponentInfo &info = componentOf(context, entity, name);
        sol::state_view lua(state);
        sol::table fields = lua.create_table();

        for (const std::string &field : info.getFieldNames()) {
            fields[field] = toLua(lua, info.getField(registry, entity, field));
        }
        return fields;
    });
    library.set_function("set", [&context](const sol::object &id, const std::string &name, const sol::object &value) {
        ecs::Registry &registry = registryOf(context);
        const ecs::Entity entity = toEntity(registry, id);
        const ecs::ComponentInfo &info = componentOf(context, entity, name);
        std::vector<std::pair<std::string, ecs::FieldValue>> fields;

        // Every field is checked before the first is written: a wrong one leaves the component unchanged
        for (const auto &[key, field] : toTable(value, "the fields of " + name)) {
            if (key.get_type() != sol::type::string) {
                throw std::invalid_argument("the fields of " + name + " must be named");
            }
            const std::string fieldName = key.as<std::string>();

            fields.emplace_back(fieldName, fromLua(field, info.getFieldType(fieldName), name + "." + fieldName));
        }
        for (const auto &[fieldName, field] : fields) {
            info.setField(registry, entity, fieldName, field);
        }
    });
}

} // namespace younity::scripting
