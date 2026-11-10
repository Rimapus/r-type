/*
** EPITECH PROJECT, 2026
** younity-engine/tests/tools/Global
** File description:
** Global metrics store unit tests
*/

#include <criterion/criterion.h>

#include "younity/tools/Global.hpp"

Test(tools_global, store_is_always_the_same)
{
    cr_assert_eq(&younity::tools::metrics(), &younity::tools::metrics());
}

Test(tools_global, records_in_the_global_store)
{
    younity::tools::metrics().clear();
    younity::tools::metrics().record("frame_time", 0.016);

    cr_assert(younity::tools::metrics().getMetric("frame_time").has_value());
}
