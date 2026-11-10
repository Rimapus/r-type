/*
** EPITECH PROJECT, 2026
** j-type/tests/server/InputGate
** File description:
** Unit tests of the inputs kept in the order of their step
*/

#include <criterion/criterion.h>

#include <cstdint>
#include <limits>

#include "InputGate.hpp"

using jtype::InputGate;

Test(server_input_gate, first_input_is_accepted)
{
    InputGate gate;

    cr_assert(gate.accept(7));
}

Test(server_input_gate, duplicated_input_is_ignored)
{
    InputGate gate;

    cr_assert(gate.accept(7));
    cr_assert_not(gate.accept(7));
}

Test(server_input_gate, older_input_is_ignored)
{
    InputGate gate;

    cr_assert(gate.accept(7));
    cr_assert_not(gate.accept(6));
    // The rejected input did not move the gate
    cr_assert(gate.accept(8));
}

Test(server_input_gate, newer_input_is_accepted)
{
    InputGate gate;

    cr_assert(gate.accept(7));
    cr_assert(gate.accept(8));
}

Test(server_input_gate, step_wraps_around)
{
    InputGate gate;

    cr_assert(gate.accept(std::numeric_limits<std::uint32_t>::max()));
    cr_assert(gate.accept(0));
    cr_assert_not(gate.accept(std::numeric_limits<std::uint32_t>::max()));
}
