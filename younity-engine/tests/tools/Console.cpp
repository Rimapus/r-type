/*
** EPITECH PROJECT, 2026
** younity-engine/tests/tools/Console
** File description:
** Console unit tests
*/

#include <criterion/criterion.h>

#include <expected>
#include <limits>
#include <optional>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

#include "younity/tools/Console.hpp"
#include "younity/tools/Global.hpp"

using younity::tools::CommandHandler;
using younity::tools::Console;
using younity::tools::ConsoleError;
using younity::tools::Variable;
using younity::tools::VariableType;
using younity::tools::VariableValue;

namespace {

CommandHandler echoHandler(void)
{
    return [](std::span<const std::string_view> arguments) -> std::expected<std::string, ConsoleError> {
        std::string output;

        for (const std::string_view argument : arguments) {
            if (output.empty() == false) {
                output.push_back(' ');
            }
            output.append(argument.data(), argument.size());
        }
        return output;
    };
}

std::string outputOf(Console &console, std::string_view line)
{
    const std::expected<std::string, ConsoleError> result = console.execute(line);

    cr_assert(result.has_value());
    return *result;
}

void assertError(const std::expected<std::string, ConsoleError> &result, std::string_view message)
{
    const std::string expected(message);

    cr_assert_not(result.has_value());
    cr_assert_str_eq(result.error().message.c_str(), expected.c_str());
}

} // namespace

Test(tools_console, a_variable_starts_at_its_default)
{
    Console console;

    console.addVariable<float>("physics.gravity", 9.81F, "Downward acceleration");

    cr_assert(console.hasVariable("physics.gravity"));
    cr_assert_not(console.hasVariable("physics.missing"));
    const std::optional<Variable> variable = console.getVariable("physics.gravity");

    cr_assert(variable.has_value());
    cr_assert_str_eq(variable->name.c_str(), "physics.gravity");
    cr_assert_str_eq(variable->description.c_str(), "Downward acceleration");
    cr_assert(variable->type == VariableType::Float);
    cr_assert_float_eq(std::get<float>(variable->value), 9.81F, 0.0001F);
    cr_assert_float_eq(std::get<float>(variable->defaultValue), 9.81F, 0.0001F);
    cr_assert_float_eq(console.getValue<float>("physics.gravity"), 9.81F, 0.0001F);
}

Test(tools_console, each_type_is_kept)
{
    Console console;

    console.addVariable<bool>("render.wireframe", true);
    console.addVariable<int>("network.port", 4242);
    console.addVariable<float>("audio.volume", 0.5F);
    console.addVariable<std::string>("app.title", std::string("Test name"));
    console.addVariable("app.subtitle", "The subtitle");

    cr_assert(console.getValue<bool>("render.wireframe"));
    cr_assert_eq(console.getValue<int>("network.port"), 4242);
    cr_assert_float_eq(console.getValue<float>("audio.volume"), 0.5F, 0.0001F);
    const std::string title = console.getValue<std::string>("app.title");
    const std::string subtitle = console.getValue<std::string>("app.subtitle");

    cr_assert_str_eq(title.c_str(), "Test name");
    cr_assert_str_eq(subtitle.c_str(), "The subtitle");
}

Test(tools_console, setting_a_variable_changes_it)
{
    Console console;

    console.addVariable<int>("count", 1);
    console.addVariable<std::string>("title", "old");
    console.setValue("count", 2);
    cr_assert_eq(console.getValue<int>("count"), 2);
    console.setValue("count", VariableValue{3});
    cr_assert_eq(console.getValue<int>("count"), 3);
    console.setValue("title", "new");
    const std::string newTitle = console.getValue<std::string>("title");

    cr_assert_str_eq(newTitle.c_str(), "new");
}

Test(tools_console, setting_a_wrong_type_throws)
{
    Console console;

    console.addVariable<int>("count", 1);
    cr_assert_throw(console.setValue("count", true), std::invalid_argument);
    cr_assert_throw(static_cast<void>(console.getValue<float>("count")), std::bad_variant_access);
}

Test(tools_console, an_unknown_variable_throws)
{
    Console console;

    cr_assert_not(console.getVariable("missing").has_value());
    cr_assert_throw(static_cast<void>(console.getValue<int>("missing")), std::out_of_range);
    cr_assert_throw(console.setValue("missing", 1), std::out_of_range);
    cr_assert_throw(console.resetVariable("missing"), std::out_of_range);
}

Test(tools_console, a_duplicate_name_throws)
{
    Console console;

    console.addVariable<int>("same", 1);
    cr_assert_throw(console.addVariable<int>("same", 2), std::invalid_argument);
    console.addCommand("run", "", echoHandler());
    cr_assert_throw(console.addCommand("same", "", echoHandler()), std::invalid_argument);
    cr_assert_throw(console.addVariable<int>("run", 1), std::invalid_argument);
}

Test(tools_console, an_invalid_name_throws)
{
    Console console;

    cr_assert_throw(console.addVariable<int>("", 1), std::invalid_argument);
    cr_assert_throw(console.addVariable<int>("has space", 1), std::invalid_argument);
    cr_assert_throw(console.addVariable<int>("has\ttab", 1), std::invalid_argument);
    cr_assert_throw(console.addCommand("", "", echoHandler()), std::invalid_argument);
    cr_assert_throw(console.addCommand("has space", "", echoHandler()), std::invalid_argument);
}

Test(tools_console, a_non_finite_default_throws)
{
    Console console;

    console.addVariable<float>("speed", 1.0F);
    cr_assert_throw(console.addVariable<float>("nan", std::numeric_limits<float>::quiet_NaN()), std::invalid_argument);
    cr_assert_throw(console.addVariable<float>("inf", std::numeric_limits<float>::infinity()), std::invalid_argument);
    cr_assert_throw(console.setValue("speed", std::numeric_limits<float>::infinity()), std::invalid_argument);
}

Test(tools_console, reset_restores_the_default)
{
    Console console;

    console.addVariable<int>("count", 7);
    console.setValue("count", 9);
    console.resetVariable("count");
    cr_assert_eq(console.getValue<int>("count"), 7);
}

Test(tools_console, reset_all_restores_every_default)
{
    Console console;

    console.addVariable<bool>("flag", false);
    console.addVariable<int>("count", 1);
    console.setValue("flag", true);
    console.setValue("count", 5);
    console.resetVariables();
    cr_assert_not(console.getValue<bool>("flag"));
    cr_assert_eq(console.getValue<int>("count"), 1);
}

Test(tools_console, variables_are_listed_sorted)
{
    Console console;

    console.addVariable<int>("z", 1, "last");
    console.addVariable<int>("a", 2, "first");
    std::vector<Variable> variables = console.getVariables();

    cr_assert_eq(variables.size(), 2);
    cr_assert_str_eq(variables[0].name.c_str(), "a");
    cr_assert_str_eq(variables[0].description.c_str(), "first");
    cr_assert_str_eq(variables[1].name.c_str(), "z");
    cr_assert_str_eq(variables[1].description.c_str(), "last");
    variables[0].value = 99;
    cr_assert_eq(console.getValue<int>("a"), 2);
}

Test(tools_console, commands_are_listed_sorted)
{
    Console console;

    console.addCommand("z", "last", echoHandler());
    console.addCommand("a", "first", echoHandler());
    const std::vector<younity::tools::CommandInfo> commands = console.getCommands();

    cr_assert(console.hasCommand("a"));
    cr_assert_not(console.hasCommand("missing"));
    cr_assert_eq(commands.size(), 2);
    cr_assert_str_eq(commands[0].name.c_str(), "a");
    cr_assert_str_eq(commands[0].description.c_str(), "first");
    cr_assert_str_eq(commands[1].name.c_str(), "z");
    cr_assert_str_eq(commands[1].description.c_str(), "last");
}

Test(tools_console, a_command_runs_with_its_arguments)
{
    Console console;
    int calls = 0;
    std::string received;

    console.addCommand(
        "move",
        "Move",
        [&calls, &received](std::span<const std::string_view> arguments) -> std::expected<std::string, ConsoleError> {
            ++calls;
            for (const std::string_view argument : arguments) {
                if (received.empty() == false) {
                    received.push_back(' ');
                }
                received.append(argument.data(), argument.size());
            }
            return std::string("moved");
        });
    const std::expected<std::string, ConsoleError> result = console.execute("move 1 2");

    cr_assert(result.has_value());
    cr_assert_str_eq(result->c_str(), "moved");
    cr_assert_eq(calls, 1);
    cr_assert_str_eq(received.c_str(), "1 2");
}

Test(tools_console, a_command_can_fail)
{
    Console console;

    console.addCommand("fail", "Always fails", [](std::span<const std::string_view>) -> std::expected<std::string, ConsoleError> {
        return std::unexpected(ConsoleError{"it failed"});
    });

    assertError(console.execute("fail"), "it failed");
}

Test(tools_console, a_command_can_be_removed)
{
    Console console;

    console.addCommand("run", "", echoHandler());
    cr_assert(console.removeCommand("run"));
    cr_assert_not(console.hasCommand("run"));
    cr_assert_not(console.removeCommand("run"));
}

Test(tools_console, a_command_handler_is_required)
{
    Console console;

    cr_assert_throw(console.addCommand("null", "", CommandHandler{}), std::invalid_argument);
}

Test(tools_console, a_command_can_clear_the_console)
{
    Console console;

    console.addVariable<int>("count", 1);
    console.addCommand(
        "reset", "Remove everything", [&console](std::span<const std::string_view>) -> std::expected<std::string, ConsoleError> {
            console.clear();
            return std::string("cleared");
        });
    const std::expected<std::string, ConsoleError> result = console.execute("reset");

    cr_assert(result.has_value());
    cr_assert_str_eq(result->c_str(), "cleared");
    cr_assert(console.getVariables().empty());
    cr_assert(console.getCommands().empty());
}

Test(tools_console, execute_prints_a_variable)
{
    Console console;

    console.addVariable<bool>("render.wireframe", false);
    console.addVariable<float>("speed", 1.5F);
    console.addVariable<std::string>("game.title", "My Game");
    const std::string wireframe = outputOf(console, "render.wireframe");
    const std::string speed = outputOf(console, "speed");
    const std::string title = outputOf(console, "game.title");

    cr_assert_str_eq(wireframe.c_str(), "render.wireframe = false");
    cr_assert_str_eq(speed.c_str(), "speed = 1.5");
    cr_assert_str_eq(title.c_str(), "game.title = \"My Game\"");
}

Test(tools_console, execute_sets_a_variable)
{
    Console console;

    console.addVariable<bool>("flag", false);
    console.addVariable<int>("count", 0);
    console.addVariable<float>("speed", 0.0F);
    console.addVariable<std::string>("title", "");
    const std::string flag = outputOf(console, "flag true");
    const std::string count = outputOf(console, "count 42");
    const std::string speed = outputOf(console, "speed 2.5");
    const std::string title = outputOf(console, "title \"My Game\"");

    cr_assert_str_eq(flag.c_str(), "flag = true");
    cr_assert_str_eq(count.c_str(), "count = 42");
    cr_assert_str_eq(speed.c_str(), "speed = 2.5");
    cr_assert_str_eq(title.c_str(), "title = \"My Game\"");
    cr_assert(console.getValue<bool>("flag"));
    cr_assert_eq(console.getValue<int>("count"), 42);
    cr_assert_float_eq(console.getValue<float>("speed"), 2.5F, 0.0001F);
    const std::string newTitle = console.getValue<std::string>("title");

    cr_assert_str_eq(newTitle.c_str(), "My Game");
}

Test(tools_console, execute_parses_each_type)
{
    Console console;

    console.addVariable<bool>("flag", false);
    console.addVariable<int>("count", 0);
    console.addVariable<float>("speed", 0.0F);
    console.addVariable<std::string>("title", "");
    outputOf(console, "flag 0");
    outputOf(console, "flag 1");
    outputOf(console, "count -12");
    outputOf(console, "speed 1.5e2");
    outputOf(console, "title \"\"");
    cr_assert(console.getValue<bool>("flag"));
    cr_assert_eq(console.getValue<int>("count"), -12);
    cr_assert_float_eq(console.getValue<float>("speed"), 150.0F, 0.0001F);
    cr_assert(console.getValue<std::string>("title").empty());
}

Test(tools_console, execute_rejects_a_bad_value)
{
    Console console;

    console.addVariable<bool>("flag", false);
    console.addVariable<int>("count", 1);
    console.addVariable<float>("speed", 1.0F);
    assertError(console.execute("flag yes"), "invalid value for flag: expected true, false, 1 or 0");
    assertError(console.execute("count 12x"), "invalid value for count: expected an integer");
    assertError(console.execute("count 99999999999999999999"), "invalid value for count: expected an integer");
    assertError(console.execute("speed nan"), "invalid value for speed: expected a finite number");
    cr_assert_not(console.getValue<bool>("flag"));
    cr_assert_eq(console.getValue<int>("count"), 1);
    cr_assert_float_eq(console.getValue<float>("speed"), 1.0F, 0.0001F);
}

Test(tools_console, execute_rejects_extra_arguments)
{
    Console console;

    console.addVariable<int>("count", 1);

    assertError(console.execute("count 1 2"), "the variable count takes one value");
    cr_assert_eq(console.getValue<int>("count"), 1);
}

Test(tools_console, execute_rejects_an_unknown_name)
{
    Console console;

    assertError(console.execute("missing"), "unknown command or variable: missing");
}

Test(tools_console, execute_parses_quoted_arguments)
{
    Console console;

    console.addCommand("say", "Echo the arguments", echoHandler());
    const std::string greeting = outputOf(console, "say \"hello world\"");
    const std::string escaped = outputOf(console, "say \"a \\\"b\\\" c\"");
    const std::string empty = outputOf(console, "say \"\"");

    cr_assert_str_eq(greeting.c_str(), "hello world");
    cr_assert_str_eq(escaped.c_str(), "a \"b\" c");
    cr_assert_str_eq(empty.c_str(), "");
}

Test(tools_console, execute_rejects_a_bad_quote)
{
    Console console;

    console.addCommand("say", "Echo the arguments", echoHandler());

    assertError(console.execute("say \"hello"), "missing closing quote");
    assertError(console.execute("say \"hello\"world"), "a quoted argument must be followed by a space");
    assertError(console.execute("say \"a\\nb\""), "invalid escape sequence");
}

Test(tools_console, execute_accepts_an_empty_line)
{
    Console console;
    const std::string output = outputOf(console, "   ");

    cr_assert_str_eq(output.c_str(), "");
}

Test(tools_console, help_lists_the_commands_and_the_variables)
{
    Console console;

    console.addCommand("spawn", "Spawn an enemy", echoHandler());
    console.addCommand("quit", "", echoHandler());
    console.addVariable<bool>("render.wireframe", false, "Draw the edges");
    const std::string output = outputOf(console, "help");

    cr_assert_str_eq(output.c_str(), "Commands:\n"
                                     "  quit\n"
                                     "  spawn - Spawn an enemy\n"
                                     "Variables:\n"
                                     "  render.wireframe = false - Draw the edges");
}

Test(tools_console, help_says_when_there_is_nothing)
{
    Console console;
    const std::string output = outputOf(console, "help");

    cr_assert_str_eq(output.c_str(), "No command or variable");
}

Test(tools_console, help_takes_no_argument)
{
    Console console;

    assertError(console.execute("help spawn"), "help takes no argument");
}

Test(tools_console, the_help_name_is_reserved)
{
    Console console;

    cr_assert_throw(console.addCommand("help", "", echoHandler()), std::invalid_argument);
    cr_assert_throw(console.addVariable<int>("help", 1), std::invalid_argument);
}

Test(tools_console, clear_removes_everything)
{
    Console console;

    console.addVariable<int>("count", 1);
    console.addCommand("run", "", echoHandler());
    console.clear();
    cr_assert_not(console.hasVariable("count"));
    cr_assert_not(console.hasCommand("run"));
    cr_assert(console.getVariables().empty());
    cr_assert(console.getCommands().empty());
}

Test(tools_console, the_global_console_is_shared)
{
    cr_assert_eq(&younity::tools::console(), &younity::tools::console());
}
