/*
** EPITECH PROJECT, 2026
** younity-engine/tests/editor/ConsoleWindow
** File description:
** ConsoleWindow unit tests
*/

#include <criterion/criterion.h>

#include <stdexcept>

#include "../TestSupport.hpp"
#include "younity/editor/ConsoleWindow.hpp"
#include "younity/logger/LoggerRegistry.hpp"
#include "younity/tools/Console.hpp"

using younity::Rectf;
using younity::editor::CONSOLE_TITLE;
using younity::editor::ConsoleWindow;
using younity::logger::LoggerRegistry;
using younity::test::createContext;
using younity::test::destroyContext;
using younity::test::drawFrame;
using younity::test::isDrawn;
using younity::test::isFixedAt;
using younity::tools::Console;

Test(editor_console_window, draws_the_console_window, .init = createContext, .fini = destroyContext)
{
    Console console;
    LoggerRegistry logs;
    ConsoleWindow window(console, logs);

    drawFrame(window);
    cr_assert(isDrawn(CONSOLE_TITLE));
}

Test(editor_console_window, draws_after_a_log_message, .init = createContext, .fini = destroyContext)
{
    Console console;
    LoggerRegistry logs;
    ConsoleWindow window(console, logs);

    logs.get("test").warn("a warning");
    drawFrame(window);
    cr_assert(isDrawn(CONSOLE_TITLE));
}

Test(editor_console_window, is_fixed_in_its_area, .init = createContext, .fini = destroyContext)
{
    Console console;
    LoggerRegistry logs;
    ConsoleWindow window(console, logs);

    ImGui::NewFrame();
    window.draw(Rectf{0.0F, 400.0F, 800.0F, 200.0F});
    ImGui::Render();
    cr_assert(isFixedAt(CONSOLE_TITLE, Rectf{0.0F, 400.0F, 800.0F, 200.0F}));
}

Test(editor_console_window, a_registry_has_one_console_window)
{
    Console console;
    LoggerRegistry logs;
    ConsoleWindow window(console, logs);

    cr_assert_throw((ConsoleWindow(console, logs)), std::invalid_argument);
}

Test(editor_console_window, destruction_releases_the_log_sink)
{
    Console console;
    LoggerRegistry logs;

    {
        ConsoleWindow window(console, logs);

        cr_assert_no_throw(logs.get("test").info("a message"), std::exception);
    }
    cr_assert_no_throw((ConsoleWindow(console, logs)), std::exception);
}

Test(editor_console_window, rejects_an_empty_scrollback)
{
    Console console;
    LoggerRegistry logs;

    cr_assert_throw((ConsoleWindow(console, logs, 0)), std::invalid_argument);
}
