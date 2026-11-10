/*
** EPITECH PROJECT, 2026
** younity-engine/modules/tools/include/younity/tools/Console
** File description:
** Console header file
*/

#pragma once

#include <concepts>
#include <expected>
#include <functional>
#include <map>
#include <optional>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

namespace younity::tools {

/**
 * @brief The type of a console variable, one of the types of VariableValue.
 */
enum class VariableType {
    /** @brief A bool. */
    Bool,
    /** @brief An int. */
    Int,
    /** @brief A float. */
    Float,
    /** @brief A std::string. */
    String,
};

/**
 * @brief The value of a console variable.
 */
using VariableValue = std::variant<bool, int, float, std::string>;

/**
 * @brief A console variable, copied when it is read.
 */
struct Variable
{
    /** @brief The name used to reach the variable. */
    std::string name;
    /** @brief What the variable controls, shown by a console. */
    std::string description;
    /** @brief The type of the value. */
    VariableType type = VariableType::Bool;
    /** @brief The current value. */
    VariableValue value = false;
    /** @brief The value put back by resetVariable. */
    VariableValue defaultValue = false;
};

/**
 * @brief The name and description of a console command.
 */
struct CommandInfo
{
    /** @brief The name used to execute the command. */
    std::string name;
    /** @brief What the command does, shown by a console. */
    std::string description;
};

/**
 * @brief Why a console line could not be executed.
 */
struct ConsoleError
{
    /** @brief The reason, ready to show to the user. */
    std::string message;
};

/**
 * @brief Handle the arguments of a console command.
 *        The views point into the executed line and are valid only while the command runs.
 */
using CommandHandler = std::function<std::expected<std::string, ConsoleError>(std::span<const std::string_view> arguments)>;

/**
 * @brief The command every console runs: it lists the commands and the variables with their descriptions.
 *        Its name is reserved, so neither a command nor a variable can take it.
 */
inline constexpr std::string_view HELP_COMMAND = "help";

/**
 * @brief Named variables and commands that the engine and the game change and run while the game runs.
 *
 * A variable and a command share one namespace. A name is case sensitive and cannot be empty, contain
 * a space or be HELP_COMMAND. The console depends on no UI, so a server uses it like a client. It has no lock:
 * use it from the thread that runs the game loop.
 *
 * A mistake in the code throws: a duplicate name, an empty name or a null command handler throws
 * std::invalid_argument, and an unknown variable throws std::out_of_range. The text given to
 * execute() comes from outside the code, so its errors are returned as a ConsoleError.
 */
class Console
{
public:
    /**
     * @brief Add a variable, e.g. addVariable<bool>("render.wireframe", false, "Draw the edges").
     *
     * @tparam T The type of the variable: bool, int, float or std::string
     * @param name The name of the variable, e.g. "render.wireframe"
     * @param defaultValue The value it starts with and resetVariable puts back
     * @param description What the variable controls, empty by default
     *
     * @throw std::invalid_argument if the name is empty, has a space or is HELP_COMMAND, if the name is
     *        already used by a variable or a command, or if a float is not finite.
     */
    template <typename T>
        requires(std::same_as<T, bool> || std::same_as<T, int> || std::same_as<T, float> || std::same_as<T, std::string>)
    void addVariable(std::string name, T defaultValue, std::string description = {})
    {
        this->addVariableImpl(std::move(name), VariableValue(std::move(defaultValue)), variableTypeOf<T>(), std::move(description));
    }

    /**
     * @brief Add a std::string variable from a string literal.
     *
     * @param name The name of the variable
     * @param defaultValue The value it starts with
     * @param description What the variable controls, empty by default
     *
     * @throw std::invalid_argument if the default value is null, or for the reasons of the
     *        template overload.
     */
    void addVariable(std::string name, const char *defaultValue, std::string description = {});

    /**
     * @brief Check if a variable has this name.
     *
     * @param name The name of the variable
     *
     * @return true if a variable has this name.
     */
    [[nodiscard]] bool hasVariable(std::string_view name) const;

    /**
     * @brief Get a copy of a variable.
     *
     * @param name The name of the variable
     *
     * @return The variable, or std::nullopt if no variable has this name.
     */
    [[nodiscard]] std::optional<Variable> getVariable(std::string_view name) const;

    /**
     * @brief Get the current value of a variable.
     *
     * @tparam T The type of the variable: bool, int, float or std::string
     * @param name The name of the variable
     *
     * @return The value.
     *
     * @throw std::out_of_range if no variable has this name.
     * @throw std::bad_variant_access if the variable has another type than T.
     */
    template <typename T>
        requires(std::same_as<T, bool> || std::same_as<T, int> || std::same_as<T, float> || std::same_as<T, std::string>)
    [[nodiscard]] T getValue(std::string_view name) const
    {
        const std::optional<Variable> variable = this->getVariable(name);

        if (variable.has_value() == false) {
            throw std::out_of_range("younity::tools::Console: the variable " + std::string(name) + " does not exist");
        }
        return std::get<T>(variable->value);
    }

    /**
     * @brief Change the value of a variable.
     *
     * @param name The name of the variable
     * @param value The new value
     *
     * @throw std::out_of_range if no variable has this name.
     * @throw std::invalid_argument if the value has another type than the variable, or if a float
     *        is not finite.
     */
    void setValue(std::string_view name, VariableValue value);

    /**
     * @brief Change the value of a variable from a typed value.
     *
     * @tparam T The type of the variable: bool, int, float or std::string
     * @param name The name of the variable
     * @param value The new value
     *
     * @throw std::out_of_range if no variable has this name.
     * @throw std::invalid_argument if the value has another type than the variable, or if a float
     *        is not finite.
     */
    template <typename T>
        requires(std::same_as<T, bool> || std::same_as<T, int> || std::same_as<T, float> || std::same_as<T, std::string>)
    void setValue(std::string_view name, T value)
    {
        this->setValue(name, VariableValue(std::move(value)));
    }

    /**
     * @brief Change the value of a std::string variable from a string literal.
     *
     * @param name The name of the variable
     * @param value The new value
     *
     * @throw std::out_of_range if no variable has this name.
     * @throw std::invalid_argument if the variable is not a string, or if the value is null.
     */
    void setValue(std::string_view name, const char *value);

    /**
     * @brief Put the default value back in a variable.
     *
     * @param name The name of the variable
     *
     * @throw std::out_of_range if no variable has this name.
     */
    void resetVariable(std::string_view name);

    /**
     * @brief Put the default value back in every variable.
     */
    void resetVariables(void);

    /**
     * @brief Get copies of all the variables.
     *
     * @return The variables, sorted by name, empty if the console has none.
     */
    [[nodiscard]] std::vector<Variable> getVariables(void) const;

    /**
     * @brief Add a command, e.g. addCommand("teleport", "Move the player", handler).
     *
     * @param name The name of the command, e.g. "teleport"
     * @param description What the command does, shown by a console
     * @param handler The function called with the arguments after the name
     *
     * @throw std::invalid_argument if the name is empty, has a space or is HELP_COMMAND, if the name is
     *        already used by a variable or a command, or if the handler is empty.
     */
    void addCommand(std::string name, std::string description, CommandHandler handler);

    /**
     * @brief Remove a command, e.g. when the object it acts on is destroyed.
     *
     * @param name The name of the command
     *
     * @return true if a command had this name.
     */
    bool removeCommand(std::string_view name);

    /**
     * @brief Check if a command has this name.
     *
     * @param name The name of the command
     *
     * @return true if a command has this name.
     */
    [[nodiscard]] bool hasCommand(std::string_view name) const;

    /**
     * @brief Get the names and descriptions of all the commands.
     *
     * @return The commands, sorted by name, empty if the console has none.
     */
    [[nodiscard]] std::vector<CommandInfo> getCommands(void) const;

    /**
     * @brief Run a line as a console would.
     *
     * A command runs its handler with the arguments that follow its name. A bare variable name
     * prints it as "name = value", and a variable name followed by one value sets it. HELP_COMMAND
     * lists the commands, then the variables with their values, one per line. Arguments are
     * separated by spaces; a double-quoted argument keeps its spaces and accepts \" and \\.
     *
     * @param line The line to run
     *
     * @return The text to show, or why the line could not be run.
     */
    [[nodiscard]] std::expected<std::string, ConsoleError> execute(std::string_view line);

    /**
     * @brief Remove every variable and command.
     */
    void clear(void);

private:
    /**
     * @brief A command and the function that runs it.
     */
    struct Command
    {
        std::string description;
        CommandHandler handler;
    };

    /**
     * @brief Get the type of a variable from its C++ type.
     */
    template <typename T>
    [[nodiscard]] static constexpr VariableType variableTypeOf(void) noexcept
    {
        if constexpr (std::same_as<T, bool>) {
            return VariableType::Bool;
        } else if constexpr (std::same_as<T, int>) {
            return VariableType::Int;
        } else if constexpr (std::same_as<T, float>) {
            return VariableType::Float;
        } else {
            return VariableType::String;
        }
    }

    /**
     * @brief Add a variable once its value and type are known.
     */
    void addVariableImpl(std::string name, VariableValue defaultValue, VariableType type, std::string description);

    /**
     * @brief Reject an empty name, a name with a space and HELP_COMMAND.
     */
    static void checkName(std::string_view name);

    /**
     * @brief Get the text of HELP_COMMAND: the commands, then the variables with their values.
     */
    [[nodiscard]] std::string help(void) const;

    std::map<std::string, Variable, std::less<>> _variables;
    std::map<std::string, Command, std::less<>> _commands;
};

} // namespace younity::tools
