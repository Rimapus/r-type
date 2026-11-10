/*
** EPITECH PROJECT, 2026
** j-type/client/src/main
** File description:
** Main source file
*/

#include <cstdint>
#include <exception>
#include <expected>
#include <optional>
#include <print>

#include "Client.hpp"
#include "jtype/common/Arguments.hpp"
#include "jtype/common/Log.hpp"
#include "younity/network/Endpoint.hpp"
#include "younity/network/SocketError.hpp"

int main(int argc, char **argv)
{
    if (argc != 3) {
        std::println(stderr, "Usage: {} <host> <port>", argv[0]);
        return 84;
    }
    const std::optional<std::uint16_t> port = jtype::parsePort(argv[2]);

    if (!port) {
        std::println(stderr, "Invalid port: {}", argv[2]);
        return 84;
    }
    // Once, before the game: a host name can wait on the network while it is looked up
    const std::expected<younity::network::Endpoint, younity::network::SocketError> server =
        younity::network::Endpoint::resolve(argv[1], *port);

    if (!server) {
        std::println(stderr, "Unknown host: {}", argv[1]);
        return 84;
    }
    try {
        jtype::Client client(*server);

        client.run();
    } catch (const std::exception &error) {
        jtype::logger().critical("The client stops: {}", error.what());
        return 84;
    }
    return 0;
}
