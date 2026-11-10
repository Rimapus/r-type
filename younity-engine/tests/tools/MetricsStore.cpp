/*
** EPITECH PROJECT, 2026
** younity-engine/tests/tools/MetricsStore
** File description:
** MetricsStore unit tests
*/

#include <criterion/criterion.h>

#include <optional>
#include <stdexcept>
#include <thread>
#include <vector>

#include "younity/tools/MetricsStore.hpp"

using younity::tools::Metric;
using younity::tools::MetricKind;
using younity::tools::MetricsStore;

namespace {

std::vector<double> historyOf(const MetricsStore &store, const char *name)
{
    const std::optional<Metric> metric = store.getMetric(name);

    cr_assert(metric.has_value());
    return metric->history;
}

} // namespace

Test(tools_metrics_store, records_a_value)
{
    MetricsStore store;

    store.record("frame_time", 0.016);

    const std::optional<Metric> metric = store.getMetric("frame_time");

    cr_assert(metric.has_value());
    cr_assert_str_eq(metric->name.c_str(), "frame_time");
    cr_assert(metric->kind == MetricKind::Value);
    cr_assert_eq(metric->history.size(), 1);
    cr_assert_float_eq(metric->last, 0.016, 0.000001);
}

Test(tools_metrics_store, keeps_the_samples_from_the_oldest_to_the_newest)
{
    MetricsStore store;

    store.record("frame_time", 1.0);
    store.record("frame_time", 2.0);
    store.record("frame_time", 3.0);

    const std::vector<double> history = historyOf(store, "frame_time");

    cr_assert_eq(history.size(), 3);
    cr_assert_float_eq(history[0], 1.0, 0.000001);
    cr_assert_float_eq(history[1], 2.0, 0.000001);
    cr_assert_float_eq(history[2], 3.0, 0.000001);
}

Test(tools_metrics_store, keeps_the_history_size_it_was_given)
{
    const MetricsStore store;
    const MetricsStore small(3);

    cr_assert_eq(store.getHistorySize(), younity::tools::DEFAULT_METRIC_HISTORY_SIZE);
    cr_assert_eq(small.getHistorySize(), 3);
}

Test(tools_metrics_store, drops_the_oldest_sample_when_the_history_is_full)
{
    MetricsStore store(3);

    store.record("frame_time", 1.0);
    store.record("frame_time", 2.0);
    store.record("frame_time", 3.0);
    store.record("frame_time", 4.0);

    const std::vector<double> history = historyOf(store, "frame_time");

    cr_assert_eq(history.size(), 3);
    cr_assert_float_eq(history[0], 2.0, 0.000001);
    cr_assert_float_eq(history[1], 3.0, 0.000001);
    cr_assert_float_eq(history[2], 4.0, 0.000001);
}

Test(tools_metrics_store, computes_the_statistics_of_the_history)
{
    MetricsStore store;

    store.record("frame_time", 1.0);
    store.record("frame_time", 3.0);
    store.record("frame_time", 5.0);

    const std::optional<Metric> metric = store.getMetric("frame_time");

    cr_assert(metric.has_value());
    cr_assert_float_eq(metric->last, 5.0, 0.000001);
    cr_assert_float_eq(metric->minimum, 1.0, 0.000001);
    cr_assert_float_eq(metric->maximum, 5.0, 0.000001);
    cr_assert_float_eq(metric->average, 3.0, 0.000001);
}

Test(tools_metrics_store, increments_a_counter_from_zero)
{
    MetricsStore store;

    store.increment("packets_lost");

    const std::optional<Metric> metric = store.getMetric("packets_lost");

    cr_assert(metric.has_value());
    cr_assert(metric->kind == MetricKind::Counter);
    cr_assert_float_eq(metric->last, 1.0, 0.000001);
}

Test(tools_metrics_store, increments_a_counter_by_an_amount)
{
    MetricsStore store;

    store.increment("bytes_received", 1500.0);
    store.increment("bytes_received", 500.0);

    const std::optional<Metric> metric = store.getMetric("bytes_received");

    cr_assert(metric.has_value());
    cr_assert_float_eq(metric->last, 2000.0, 0.000001);
    cr_assert_eq(metric->history.size(), 2);
    cr_assert_float_eq(metric->history[0], 1500.0, 0.000001);
    cr_assert_float_eq(metric->history[1], 2000.0, 0.000001);
}

Test(tools_metrics_store, records_a_timer)
{
    MetricsStore store;

    store.record("physics", 0.002, MetricKind::Timer);

    const std::optional<Metric> metric = store.getMetric("physics");

    cr_assert(metric.has_value());
    cr_assert(metric->kind == MetricKind::Timer);
    cr_assert_float_eq(metric->last, 0.002, 0.000001);
}

Test(tools_metrics_store, returns_no_metric_for_an_unknown_name)
{
    const MetricsStore store;

    cr_assert_not(store.getMetric("unknown").has_value());
}

Test(tools_metrics_store, lists_the_metrics_by_name)
{
    MetricsStore store;

    store.record("z", 1.0);
    store.record("a", 2.0);

    const std::vector<Metric> metrics = store.getMetrics();

    cr_assert_eq(metrics.size(), 2);
    cr_assert_str_eq(metrics[0].name.c_str(), "a");
    cr_assert_str_eq(metrics[1].name.c_str(), "z");
}

Test(tools_metrics_store, clears_every_metric)
{
    MetricsStore store;

    store.record("frame_time", 0.016);
    store.increment("packets_lost");
    store.clear();

    cr_assert(store.getMetrics().empty());
    cr_assert_not(store.getMetric("frame_time").has_value());
}

Test(tools_metrics_store, refuses_a_history_size_of_zero)
{
    cr_assert_throw(MetricsStore(0), std::invalid_argument);
}

Test(tools_metrics_store, refuses_an_empty_name)
{
    MetricsStore store;

    cr_assert_throw(store.record("", 1.0), std::invalid_argument);
    cr_assert_throw(store.increment(""), std::invalid_argument);
}

Test(tools_metrics_store, records_from_several_threads)
{
    MetricsStore store;
    std::vector<std::thread> threads;

    for (int thread = 0; thread < 4; ++thread) {
        threads.emplace_back([&store] {
            for (int i = 0; i < 1000; ++i) {
                store.increment("counter");
            }
        });
    }
    for (std::thread &thread : threads) {
        thread.join();
    }

    const std::optional<Metric> metric = store.getMetric("counter");

    cr_assert(metric.has_value());
    cr_assert_float_eq(metric->last, 4000.0, 0.000001);
}
