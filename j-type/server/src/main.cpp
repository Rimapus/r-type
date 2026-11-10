/*
** EPITECH PROJECT, 2026
** j-type/server/src/main
** File description:
** Main source file
*/

#include <cstdint>
#include <exception>
#include <optional>
#include <print>

#include "Server.hpp"
#include "Signals.hpp"
#include "jtype/common/Arguments.hpp"
#include "jtype/common/Difficulty.hpp"
#include "jtype/common/Log.hpp"

int main(int argc, char **argv)
{
    if (argc != 2 && argc != 3) {
        std::println(stderr, "Usage: {} <port> [easy|normal|hard]", argv[0]);
        return 84;
    }
    const std::optional<std::uint16_t> port = jtype::parsePort(argv[1]);
    const std::optional<jtype::Difficulty> difficulty =
        argc == 3 ? jtype::parseDifficulty(argv[2]) : jtype::Difficulty::Normal;

    if (!port) {
        std::println(stderr, "Invalid port: {}", argv[1]);
        return 84;
    }
    if (!difficulty) {
        std::println(stderr, "Invalid difficulty: {}, expected easy, normal or hard", argv[2]);
        return 84;
    }
    try {
        jtype::Server server(*port, *difficulty);

        jtype::installStopHandlers();
        server.run();
    } catch (const std::exception &error) {
        jtype::logger().critical("The server stops: {}", error.what());
        return 84;
    }
    return 0;
}
