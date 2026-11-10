/*
** EPITECH PROJECT, 2026
** younity-engine/tests/editor/Log
** File description:
** Generated editor Log unit tests
*/

#include <criterion/criterion.h>

#include "younity/editor/Log.hpp"
#include "younity/logger/Global.hpp"

Test(editor_log, logger_is_named_after_the_module)
{
    cr_assert(younity::editor::logger().getName() == "editor");
}

Test(editor_log, logger_is_the_one_of_the_global_registry)
{
    cr_assert_eq(&younity::editor::logger(), &younity::logger::get("editor"));
}

Test(editor_log, logger_is_always_the_same)
{
    cr_assert_eq(&younity::editor::logger(), &younity::editor::logger());
}
