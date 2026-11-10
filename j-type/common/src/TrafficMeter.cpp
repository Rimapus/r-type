/*
** EPITECH PROJECT, 2026
** j-type/common/src/TrafficMeter
** File description:
** TrafficMeter source file
*/

#include "jtype/common/TrafficMeter.hpp"

namespace jtype {

TrafficMeter::TrafficMeter(std::chrono::steady_clock::duration period, younity::tools::MetricsStore &metrics)
    : _period(period),
      _metrics(metrics)
{
}

void TrafficMeter::addSent(std::size_t bytes) noexcept
{
    this->_sent += bytes;
}

void TrafficMeter::addReceived(std::size_t bytes) noexcept
{
    this->_received += bytes;
}

void TrafficMeter::addSnapshot(void) noexcept
{
    ++this->_snapshots;
}

std::optional<TrafficRates> TrafficMeter::update(std::chrono::steady_clock::time_point now)
{
    if (!this->_start) {
        this->_start = now;
        return std::nullopt;
    }
    const std::chrono::duration<double> elapsed = now - *this->_start;

    if (elapsed < this->_period) {
        return std::nullopt;
    }
    const TrafficRates rates{static_cast<double>(this->_sent) / elapsed.count(), static_cast<double>(this->_received) / elapsed.count(),
                             static_cast<double>(this->_snapshots) / elapsed.count()};

    this->_metrics.record("network.bytes_sent", rates.bytesSent);
    this->_metrics.record("network.bytes_received", rates.bytesReceived);
    this->_metrics.record("network.snapshots", rates.snapshots);
    this->_start = now;
    this->_sent = 0;
    this->_received = 0;
    this->_snapshots = 0;
    return rates;
}

} // namespace jtype
