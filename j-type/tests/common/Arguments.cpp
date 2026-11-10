/*
** EPITECH PROJECT, 2026
** j-type/tests/common/Arguments
** File description:
** Arguments unit tests
*/

#include <criterion/criterion.h>

#include "jtype/common/Arguments.hpp"

Test(parse_port, reads_a_valid_port)
{
    cr_assert(jtype::parsePort("4242") == 4242);
    cr_assert(jtype::parsePort("1") == 1);
    cr_assert(jtype::parsePort("65535") == 65535);
}

Test(parse_port, refuses_zero_and_out_of_range)
{
    cr_assert(!jtype::parsePort("0"));
    cr_assert(!jtype::parsePort("65536"));
    cr_assert(!jtype::parsePort("-1"));
}

Test(parse_port, refuses_what_is_not_only_a_number)
{
    cr_assert(!jtype::parsePort(""));
    cr_assert(!jtype::parsePort("abc"));
    cr_assert(!jtype::parsePort("42abc"));
    cr_assert(!jtype::parsePort(" 42"));
}
