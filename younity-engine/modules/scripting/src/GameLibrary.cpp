/*
** EPITECH PROJECT, 2026
** younity-engine/modules/scripting/src/GameLibrary
** File description:
** GameLibrary source file
*/

#include "GameLibrary.hpp"
#include "AssetPath.hpp"

#include <cmath>
#include <stdexcept>
#include <string>

namespace younity::scripting {

void openGameLibrary(sol::state &lua, const Game &game)
{
    sol::table library = lua.create_named_table("game");

    library.set_function("loadScene", [load = game.loadScene](const std::string &path) {
        checkAssetPath(path, "the scene");
        if (load) {
            load(path);
        }
    });
    library.set_function("quit", [quit = game.quit] {
        if (quit) {
            quit();
        }
    });
    library.set_function("setTimeScale", [set = game.setTimeScale](double scale) {
        if (!std::isfinite(scale) || scale < 0) {
            throw std::invalid_argument("the time scale must be a finite number from 0");
        }
        if (set) {
            set(static_cast<float>(scale));
        }
    });
}

} // namespace younity::scripting
