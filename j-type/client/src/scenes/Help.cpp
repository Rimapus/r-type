/*
** EPITECH PROJECT, 2026
** j-type/client/src/scenes/Help
** File description:
** Help source file
*/

#include "scenes/Help.hpp"

#include <cstddef>

namespace jtype {

std::vector<std::string> getHelpLines(const Bindings &bindings)
{
    std::vector<std::string> lines{
        "SHOOT THE BYDOS AND DODGE THEIR SHOTS: A HIT COSTS A LIFE.",
        "THE GAME ENDS WHEN ALL SHIPS ARE OUT OF LIVES OR THE BOSS FALLS.",
        "",
    };

    for (std::size_t action = 0; action < GAME_ACTION_COUNT; ++action) {
        lines.push_back(formatBinding(static_cast<GameAction>(action), bindings[action]));
    }
    // The keys of the menus and of the layouts cannot be bound: written as they are
    lines.insert(lines.end(), {
                                  "",
                                  "MENUS: ARROWS OR LEFT STICK TO MOVE",
                                  "ENTER OR B0 TO CONFIRM, ESCAPE OR B1 TO GO BACK",
                                  "F1: DEBUG WINDOWS    F2: EDITOR",
                                  "SETTINGS, THEN CONTROLS, BINDS OTHER KEYS.",
                              });
    return lines;
}

} // namespace jtype
