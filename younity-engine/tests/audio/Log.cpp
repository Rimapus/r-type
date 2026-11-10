/*
** EPITECH PROJECT, 2026
** younity-engine/tests/audio/Log
** File description:
** Generated audio Log unit tests
*/

#include <criterion/criterion.h>

#include "younity/audio/Log.hpp"
#include "younity/logger/Global.hpp"

Test(audio_log, logger_is_named_after_the_module)
{
    cr_assert(younity::audio::logger().getName() == "audio");
}

Test(audio_log, logger_is_the_one_of_the_global_registry)
{
    cr_assert_eq(&younity::audio::logger(), &younity::logger::get("audio"));
}

Test(audio_log, logger_is_always_the_same)
{
    cr_assert_eq(&younity::audio::logger(), &younity::audio::logger());
}
