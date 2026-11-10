/*
** EPITECH PROJECT, 2026
** younity-engine/modules/network/include/younity/network/Datagram
** File description:
** Datagram header file
*/

#pragma once

#include <cstddef>
#include <vector>

#include "younity/network/Endpoint.hpp"

namespace younity::network {

/**
 * @brief Bytes received from one peer, with who sent them.
 *
 * Plain value: the receive thread fills it, the game loop reads it.
 * The bytes hold one whole datagram, to give to decodePacket.
 */
struct ReceivedDatagram
{
    Endpoint from = Endpoint::loopback(0);
    std::vector<std::byte> bytes;
};

/**
 * @brief Bytes to send to one peer.
 *
 * Plain value: the game loop fills it, the send thread sends it
 * with UdpSocket::sendTo. The bytes hold one whole datagram,
 * built with Writer and encodePacket.
 */
struct SendRequest
{
    Endpoint to = Endpoint::loopback(0);
    std::vector<std::byte> bytes;
};

} // namespace younity::network
