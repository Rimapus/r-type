/*
** EPITECH PROJECT, 2026
** younity-engine/tests/scripting/Log
** File description:
** Generated scripting Log unit tests
*/

#include <criterion/criterion.h>

#include "younity/logger/Global.hpp"
#include "younity/scripting/Log.hpp"

Test(scripting_log, logger_is_named_after_the_module)
{
    cr_assert(younity::scripting::logger().getName() == "scripting");
}

Test(scripting_log, logger_is_the_one_of_the_global_registry)
{
    cr_assert_eq(&younity::scripting::logger(), &younity::logger::get("scripting"));
}

Test(scripting_log, logger_is_always_the_same)
{
    cr_assert_eq(&younity::scripting::logger(), &younity::scripting::logger());
}
