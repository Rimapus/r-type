/*
** EPITECH PROJECT, 2026
** j-type/tests/common/TrafficMeter
** File description:
** TrafficMeter unit tests
*/

#include <criterion/criterion.h>

#include <chrono>
#include <optional>

#include "jtype/common/TrafficMeter.hpp"
#include "younity/tools/MetricsStore.hpp"

using jtype::TrafficMeter;
using jtype::TrafficRates;
using std::chrono::milliseconds;
using std::chrono::seconds;
using younity::tools::MetricsStore;

namespace {

const std::chrono::steady_clock::time_point START{};

} // namespace

Test(common_traffic_meter, publishes_the_rates_once_the_period_ended)
{
    MetricsStore metrics;
    TrafficMeter traffic(seconds(1), metrics);

    cr_assert_not(traffic.update(START).has_value());
    traffic.addSent(100);
    traffic.addSent(100);
    traffic.addReceived(50);
    traffic.addSnapshot();
    traffic.addSnapshot();
    traffic.addSnapshot();
    cr_assert_not(traffic.update(START + milliseconds(500)).has_value());

    // Two seconds passed: the counts are averaged over them
    const std::optional<TrafficRates> rates = traffic.update(START + seconds(2));

    cr_assert(rates.has_value());
    cr_assert_float_eq(rates->bytesSent, 100.0, 1e-9);
    cr_assert_float_eq(rates->bytesReceived, 25.0, 1e-9);
    cr_assert_float_eq(rates->snapshots, 1.5, 1e-9);
    cr_assert_float_eq(metrics.getMetric("network.bytes_sent")->last, 100.0, 1e-9);
    cr_assert_float_eq(metrics.getMetric("network.bytes_received")->last, 25.0, 1e-9);
    cr_assert_float_eq(metrics.getMetric("network.snapshots")->last, 1.5, 1e-9);
}

Test(common_traffic_meter, next_period_starts_from_nothing)
{
    MetricsStore metrics;
    TrafficMeter traffic(seconds(1), metrics);

    (void)traffic.update(START);
    traffic.addReceived(1000);
    (void)traffic.update(START + seconds(1));

    const std::optional<TrafficRates> rates = traffic.update(START + seconds(2));

    cr_assert(rates.has_value());
    cr_assert_float_eq(rates->bytesReceived, 0.0, 1e-9);
    cr_assert_eq(metrics.getMetric("network.bytes_received")->history.size(), 2U);
}
