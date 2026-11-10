/*
** EPITECH PROJECT, 2026
** j-type/common/include/jtype/common/TrafficMeter
** File description:
** TrafficMeter header file
*/

#pragma once

#include <chrono>
#include <cstddef>
#include <optional>

#include "younity/tools/Global.hpp"
#include "younity/tools/MetricsStore.hpp"

namespace jtype {

/**
 * @brief The traffic of a period: what a client or the server sent and received, per second.
 */
struct TrafficRates
{
    // The bytes of the datagrams, without the 28 bytes of UDP and IPv4 headers each one also costs
    double bytesSent = 0.0;
    double bytesReceived = 0.0;
    // The whole snapshots received, their chunks put back together
    double snapshots = 0.0;
};

/**
 * @brief Counts the bytes sent and received and the snapshots received, and at the end of each period publishes their
 *        rates per second to a metrics store: network.bytes_sent, network.bytes_received and network.snapshots.
 *        The client publishes them each second for the metrics window of the debug layout, the server every ten seconds
 *        for its log.
 */
class TrafficMeter
{
public:
    /**
     * @brief Start counting, the first period starting at the first update.
     *
     * @param period The length of a period
     * @param metrics The store the rates are published to, the one shared by the engine and the game by default; it
     *                must outlive the meter
     */
    explicit TrafficMeter(std::chrono::steady_clock::duration period = std::chrono::seconds(1),
                          younity::tools::MetricsStore &metrics = younity::tools::metrics());

    /**
     * @brief Count a datagram sent.
     *
     * @param bytes Its size
     */
    void addSent(std::size_t bytes) noexcept;

    /**
     * @brief Count a datagram received.
     *
     * @param bytes Its size
     */
    void addReceived(std::size_t bytes) noexcept;

    /**
     * @brief Count a whole snapshot received.
     */
    void addSnapshot(void) noexcept;

    /**
     * @brief End the period once it lasted long enough: publish its rates and start the next one.
     *
     * @param now The time of the call, e.g. std::chrono::steady_clock::now()
     *
     * @return The rates of the period that just ended, or nothing while it goes on.
     */
    std::optional<TrafficRates> update(std::chrono::steady_clock::time_point now);

private:
    std::chrono::steady_clock::duration _period;
    younity::tools::MetricsStore &_metrics;
    // Empty until the first update, which starts the first period
    std::optional<std::chrono::steady_clock::time_point> _start;
    std::size_t _sent = 0;
    std::size_t _received = 0;
    std::size_t _snapshots = 0;
};

} // namespace jtype
