/*
** EPITECH PROJECT, 2026
** younity-engine/tests/core/Log
** File description:
** Generated core Log unit tests
*/

#include <criterion/criterion.h>

#include "younity/core/Log.hpp"
#include "younity/logger/Global.hpp"

Test(core_log, logger_is_named_after_the_module)
{
    cr_assert(younity::core::logger().getName() == "core");
}

Test(core_log, logger_is_the_one_of_the_global_registry)
{
    cr_assert_eq(&younity::core::logger(), &younity::logger::get("core"));
}

Test(core_log, logger_is_always_the_same)
{
    cr_assert_eq(&younity::core::logger(), &younity::core::logger());
}
