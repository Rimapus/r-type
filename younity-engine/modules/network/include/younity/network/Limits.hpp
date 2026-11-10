/*
** EPITECH PROJECT, 2026
** younity-engine/modules/network/include/younity/network/Limits
** File description:
** Network buffer limits header file
*/

#pragma once

#include <chrono>
#include <cstddef>

namespace younity::network {

/**
 * @brief Largest accepted datagram payload, in bytes.
 *
 * Stays under the usual 1500 bytes Ethernet MTU once IP and UDP headers are
 * accounted for, so packets avoid fragmentation on LANs, VPNs and tunnels.
 */
inline constexpr std::size_t MAX_PACKET_BYTES = 1200;

/**
 * @brief Largest accepted string field, in bytes.
 *
 * Bounds the allocation triggered by a length prefix, so a malicious length
 * cannot force a huge allocation.
 */
inline constexpr std::size_t MAX_STRING_BYTES = 256;

/**
 * @brief Default bound of a queue, in values.
 *
 * Bounds the memory a stalled consumer lets a producer reserve: pushing to a
 * full queue is refused instead of growing it. UdpPump uses it for both the
 * received and the unsent datagrams.
 */
inline constexpr std::size_t DEFAULT_QUEUE_CAPACITY = 1024;

/**
 * @brief Default delay between two heartbeats of a session.
 *
 * Several heartbeats fit in one session timeout, so a single lost datagram
 * never drops a healthy session.
 */
inline constexpr std::chrono::milliseconds DEFAULT_HEARTBEAT_INTERVAL{500};

/**
 * @brief Default delay between two pings of a session.
 *
 * Each ping measures a round trip, published as the "network.latency" metric.
 */
inline constexpr std::chrono::milliseconds DEFAULT_PING_INTERVAL{1000};

/**
 * @brief Default delay without any packet from a peer before its session ends.
 */
inline constexpr std::chrono::milliseconds DEFAULT_SESSION_TIMEOUT{3000};

/**
 * @brief Largest number of peers a SessionTable keeps at once.
 *
 * A Connect from a new peer is refused while the table is full, so an
 * endpoint scan cannot grow the table without limit.
 */
inline constexpr std::size_t MAX_SESSIONS = 1024;

} // namespace younity::network
