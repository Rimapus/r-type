/*
** EPITECH PROJECT, 2026
** j-climb/src/main
** File description:
** Main source file
*/

#include <exception>
#include <filesystem>
#include <print>

#include "Game.hpp"

int main(int argc, char **argv)
{
    if (argc > 2) {
        std::println(stderr, "Usage: {} [assets folder]", argv[0]);
        return 84;
    }
    // J-Type starts the game from its own folder, so it gives the assets folder of J-Climb
    const std::filesystem::path assets = argc == 2 ? argv[1] : "assets";

    try {
        jclimb::Game game(assets);

        game.run();
    } catch (const std::exception &error) {
        std::println(stderr, "J-Climb stops: {}", error.what());
        return 84;
    }
    return 0;
}
