/*
** EPITECH PROJECT, 2026
** younity-engine/modules/tools/src/Console
** File description:
** Console source file
*/

#include "younity/tools/Console.hpp"

#include <cctype>
#include <charconv>
#include <cmath>
#include <cstddef>
#include <optional>
#include <stdexcept>
#include <system_error>
#include <utility>
#include <vector>

namespace younity::tools {

namespace {

bool isSpace(char character) noexcept
{
    return std::isspace(static_cast<unsigned char>(character)) != 0;
}

std::string escapeString(std::string_view value)
{
    std::string escaped;

    escaped.reserve(value.size() + 2);
    escaped.push_back('"');
    for (const char character : value) {
        if (character == '"' || character == '\\') {
            escaped.push_back('\\');
        }
        escaped.push_back(character);
    }
    escaped.push_back('"');
    return escaped;
}

std::string valueToString(const Variable &variable)
{
    switch (variable.type) {
    case VariableType::Bool:
        return std::get<bool>(variable.value) ? "true" : "false";
    case VariableType::Int:
        return std::to_string(std::get<int>(variable.value));
    case VariableType::Float: {
        char buffer[64] = {};
        const float value = std::get<float>(variable.value);
        const std::to_chars_result result = std::to_chars(buffer, buffer + sizeof(buffer), value, std::chars_format::general);

        return std::string(buffer, result.ptr);
    }
    case VariableType::String:
        return escapeString(std::get<std::string>(variable.value));
    }
    return {};
}

std::string_view typeName(VariableType type) noexcept
{
    switch (type) {
    case VariableType::Bool:
        return "bool";
    case VariableType::Int:
        return "int";
    case VariableType::Float:
        return "float";
    case VariableType::String:
        return "string";
    }
    return "value";
}

bool sameType(const VariableValue &value, VariableType type) noexcept
{
    switch (type) {
    case VariableType::Bool:
        return std::holds_alternative<bool>(value);
    case VariableType::Int:
        return std::holds_alternative<int>(value);
    case VariableType::Float:
        return std::holds_alternative<float>(value);
    case VariableType::String:
        return std::holds_alternative<std::string>(value);
    }
    return false;
}

std::expected<std::vector<std::string>, ConsoleError> tokenize(std::string_view line)
{
    std::vector<std::string> tokens;
    std::size_t index = 0;

    while (index < line.size()) {
        while (index < line.size() && isSpace(line[index])) {
            ++index;
        }
        if (index == line.size()) {
            break;
        }
        std::string token;

        if (line[index] == '"') {
            ++index;
            bool closed = false;

            while (index < line.size()) {
                const char character = line[index++];

                if (character == '"') {
                    closed = true;
                    break;
                }
                if (character == '\\') {
                    if (index == line.size()) {
                        return std::unexpected(ConsoleError{"missing closing quote"});
                    }
                    const char escaped = line[index++];

                    if (escaped != '"' && escaped != '\\') {
                        return std::unexpected(ConsoleError{"invalid escape sequence"});
                    }
                    token.push_back(escaped);
                    continue;
                }
                token.push_back(character);
            }
            if (closed == false) {
                return std::unexpected(ConsoleError{"missing closing quote"});
            }
            if (index < line.size() && isSpace(line[index]) == false) {
                return std::unexpected(ConsoleError{"a quoted argument must be followed by a space"});
            }
        } else {
            while (index < line.size() && isSpace(line[index]) == false) {
                token.push_back(line[index++]);
            }
        }
        tokens.push_back(std::move(token));
    }
    return tokens;
}

std::expected<VariableValue, ConsoleError> parseValue(VariableType type, std::string_view text)
{
    switch (type) {
    case VariableType::Bool:
        if (text == "true" || text == "1") {
            return VariableValue(true);
        }
        if (text == "false" || text == "0") {
            return VariableValue(false);
        }
        return std::unexpected(ConsoleError{"expected true, false, 1 or 0"});
    case VariableType::Int: {
        int value = 0;
        const std::from_chars_result result = std::from_chars(text.data(), text.data() + text.size(), value);

        if (result.ec != std::errc{} || result.ptr != text.data() + text.size()) {
            return std::unexpected(ConsoleError{"expected an integer"});
        }
        return VariableValue(value);
    }
    case VariableType::Float: {
        float value = 0.0F;
        const std::from_chars_result result = std::from_chars(text.data(), text.data() + text.size(), value, std::chars_format::general);

        if (result.ec != std::errc{} || result.ptr != text.data() + text.size() || std::isfinite(value) == false) {
            return std::unexpected(ConsoleError{"expected a finite number"});
        }
        return VariableValue(value);
    }
    case VariableType::String:
        return VariableValue(std::string(text));
    }
    return std::unexpected(ConsoleError{"unknown variable type"});
}

/**
 * @brief Write one line of the help: the entry, then its description when it has one.
 *
 * @param entry The name of a command, or "name = value" for a variable
 * @param description What it does, possibly empty
 *
 * @return The line, indented under its section.
 */
std::string helpLine(std::string_view entry, std::string_view description)
{
    std::string line = "  " + std::string(entry);

    if (!description.empty()) {
        line += " - " + std::string(description);
    }
    return line;
}

} // namespace

void Console::addVariable(std::string name, const char *defaultValue, std::string description)
{
    if (defaultValue == nullptr) {
        throw std::invalid_argument("younity::tools::Console: the default value is null");
    }
    this->addVariable(std::move(name), std::string(defaultValue), std::move(description));
}

void Console::addVariableImpl(std::string name, VariableValue defaultValue, VariableType type, std::string description)
{
    this->checkName(name);
    if (this->_variables.contains(name) || this->_commands.contains(name)) {
        throw std::invalid_argument("younity::tools::Console: the name " + name + " is already used");
    }
    if (sameType(defaultValue, type) == false) {
        throw std::invalid_argument("younity::tools::Console: the variable " + name + " has a value of another type");
    }
    if (type == VariableType::Float && std::isfinite(std::get<float>(defaultValue)) == false) {
        throw std::invalid_argument("younity::tools::Console: the variable " + name + " has a non-finite default");
    }
    VariableValue current = defaultValue;
    Variable variable{std::move(name), std::move(description), type, std::move(current), std::move(defaultValue)};

    this->_variables.emplace(variable.name, std::move(variable));
}

void Console::checkName(std::string_view name)
{
    if (name.empty()) {
        throw std::invalid_argument("younity::tools::Console: the name is empty");
    }
    if (name == HELP_COMMAND) {
        throw std::invalid_argument("younity::tools::Console: the name " + std::string(name) + " is reserved");
    }
    for (const char character : name) {
        if (isSpace(character)) {
            throw std::invalid_argument("younity::tools::Console: the name " + std::string(name) + " has a space");
        }
    }
}

bool Console::hasVariable(std::string_view name) const
{
    return this->_variables.contains(name);
}

std::optional<Variable> Console::getVariable(std::string_view name) const
{
    const auto variable = this->_variables.find(name);

    if (variable == this->_variables.end()) {
        return std::nullopt;
    }
    return variable->second;
}

void Console::setValue(std::string_view name, VariableValue value)
{
    const auto variable = this->_variables.find(name);

    if (variable == this->_variables.end()) {
        throw std::out_of_range("younity::tools::Console: the variable " + std::string(name) + " does not exist");
    }
    if (sameType(value, variable->second.type) == false) {
        throw std::invalid_argument(
            "younity::tools::Console: the variable " + variable->second.name + " is a " + std::string(typeName(variable->second.type)));
    }
    if (variable->second.type == VariableType::Float && std::isfinite(std::get<float>(value)) == false) {
        throw std::invalid_argument("younity::tools::Console: the variable " + variable->second.name + " must be finite");
    }
    variable->second.value = std::move(value);
}

void Console::setValue(std::string_view name, const char *value)
{
    if (value == nullptr) {
        throw std::invalid_argument("younity::tools::Console: the value is null");
    }
    this->setValue(name, std::string(value));
}

void Console::resetVariable(std::string_view name)
{
    const auto variable = this->_variables.find(name);

    if (variable == this->_variables.end()) {
        throw std::out_of_range("younity::tools::Console: the variable " + std::string(name) + " does not exist");
    }
    variable->second.value = variable->second.defaultValue;
}

void Console::resetVariables(void)
{
    for (auto &[name, variable] : this->_variables) {
        (void)name;
        variable.value = variable.defaultValue;
    }
}

std::vector<Variable> Console::getVariables(void) const
{
    std::vector<Variable> variables;

    variables.reserve(this->_variables.size());
    for (const auto &[name, variable] : this->_variables) {
        (void)name;
        variables.push_back(variable);
    }
    return variables;
}

void Console::addCommand(std::string name, std::string description, CommandHandler handler)
{
    this->checkName(name);
    if (!handler) {
        throw std::invalid_argument("younity::tools::Console: the handler is empty");
    }
    if (this->_variables.contains(name) || this->_commands.contains(name)) {
        throw std::invalid_argument("younity::tools::Console: the name " + name + " is already used");
    }
    this->_commands.emplace(std::move(name), Command{std::move(description), std::move(handler)});
}

bool Console::removeCommand(std::string_view name)
{
    const auto command = this->_commands.find(name);

    if (command == this->_commands.end()) {
        return false;
    }
    this->_commands.erase(command);
    return true;
}

bool Console::hasCommand(std::string_view name) const
{
    return this->_commands.contains(name);
}

std::vector<CommandInfo> Console::getCommands(void) const
{
    std::vector<CommandInfo> commands;

    commands.reserve(this->_commands.size());
    for (const auto &[name, command] : this->_commands) {
        commands.push_back(CommandInfo{name, command.description});
    }
    return commands;
}

std::expected<std::string, ConsoleError> Console::execute(std::string_view line)
{
    std::expected<std::vector<std::string>, ConsoleError> tokenized = tokenize(line);

    if (tokenized.has_value() == false) {
        return std::unexpected(std::move(tokenized.error()));
    }
    const std::vector<std::string> &tokens = *tokenized;

    if (tokens.empty()) {
        return std::string{};
    }
    std::vector<std::string_view> views;

    views.reserve(tokens.size());
    for (const std::string &token : tokens) {
        views.push_back(token);
    }
    const std::string_view name = views.front();
    const std::span<const std::string_view> arguments(views.data() + 1, views.size() - 1);

    if (name == HELP_COMMAND) {
        if (!arguments.empty()) {
            return std::unexpected(ConsoleError{std::string(HELP_COMMAND) + " takes no argument"});
        }
        return this->help();
    }
    if (const auto command = this->_commands.find(name); command != this->_commands.end()) {
        // Copy the handler so a command that clears the console does not destroy itself while it runs.
        const CommandHandler handler = command->second.handler;

        return handler(arguments);
    }
    if (const auto variable = this->_variables.find(name); variable != this->_variables.end()) {
        if (arguments.empty()) {
            return variable->second.name + " = " + valueToString(variable->second);
        }
        if (arguments.size() > 1) {
            return std::unexpected(ConsoleError{"the variable " + variable->second.name + " takes one value"});
        }
        std::expected<VariableValue, ConsoleError> value = parseValue(variable->second.type, arguments.front());

        if (value.has_value() == false) {
            return std::unexpected(ConsoleError{"invalid value for " + variable->second.name + ": " + value.error().message});
        }
        this->setValue(variable->first, std::move(*value));
        return variable->second.name + " = " + valueToString(variable->second);
    }
    return std::unexpected(ConsoleError{"unknown command or variable: " + std::string(name)});
}

std::string Console::help(void) const
{
    if (this->_commands.empty() && this->_variables.empty()) {
        return "No command or variable";
    }
    std::string text;

    if (!this->_commands.empty()) {
        text += "Commands:";
        for (const auto &[name, command] : this->_commands) {
            text += "\n" + helpLine(name, command.description);
        }
    }
    if (!this->_variables.empty()) {
        text += text.empty() ? "Variables:" : "\nVariables:";
        for (const auto &[name, variable] : this->_variables) {
            text += "\n" + helpLine(name + " = " + valueToString(variable), variable.description);
        }
    }
    return text;
}

void Console::clear(void)
{
    this->_variables.clear();
    this->_commands.clear();
}

} // namespace younity::tools
