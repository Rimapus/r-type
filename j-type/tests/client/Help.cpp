/*
** EPITECH PROJECT, 2026
** j-type/tests/client/Help
** File description:
** Help unit tests
*/

#include <criterion/criterion.h>

#include <algorithm>
#include <string>
#include <vector>

#include "input/Bindings.hpp"
#include "jtype/common/Constants.hpp"
#include "render/BitmapFont.hpp"
#include "scenes/Help.hpp"
#include "younity/input/Key.hpp"

using jtype::Bindings;
using jtype::GameAction;

namespace {

bool contains(const std::vector<std::string> &lines, const std::string &text)
{
    return std::ranges::find(lines, text) != lines.end();
}

} // namespace

Test(client_help, recalls_the_default_controls)
{
    const std::vector<std::string> lines = jtype::getHelpLines(jtype::getDefaultBindings());

    cr_assert(contains(lines, "SHOOT: SPACE / B0"));
    cr_assert(contains(lines, "F1: DEBUG WINDOWS    F2: EDITOR"));
}

Test(client_help, shows_the_keys_the_player_chose)
{
    Bindings bindings = jtype::getDefaultBindings();

    jtype::rebindKey(bindings, GameAction::Shoot, younity::input::Key::X);
    cr_assert(contains(jtype::getHelpLines(bindings), "SHOOT: X / B0"));
}

Test(client_help, every_line_fits_on_the_screen)
{
    for (const std::string &line : jtype::getHelpLines(jtype::getDefaultBindings())) {
        cr_assert_leq(static_cast<float>(line.size()) * jtype::GLYPH_ADVANCE, jtype::WORLD_WIDTH, "%s", line.c_str());
    }
}
