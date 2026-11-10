/*
** EPITECH PROJECT, 2026
** younity-engine/tests/render/Log
** File description:
** Generated render Log unit tests
*/

#include <criterion/criterion.h>

#include "younity/logger/Global.hpp"
#include "younity/render/Log.hpp"

Test(render_log, logger_is_named_after_the_module)
{
    cr_assert(younity::render::logger().getName() == "render");
}

Test(render_log, logger_is_the_one_of_the_global_registry)
{
    cr_assert_eq(&younity::render::logger(), &younity::logger::get("render"));
}

Test(render_log, logger_is_always_the_same)
{
    cr_assert_eq(&younity::render::logger(), &younity::render::logger());
}
