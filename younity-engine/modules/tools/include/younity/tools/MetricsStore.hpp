/*
** EPITECH PROJECT, 2026
** younity-engine/modules/tools/include/younity/tools/MetricsStore
** File description:
** MetricsStore header file
*/

#pragma once

#include <cstddef>
#include <deque>
#include <map>
#include <mutex>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "younity/tools/Metric.hpp"

namespace younity::tools {

/**
 * @brief Number of samples kept for each metric by default: two seconds at 60 frames per second.
 */
inline constexpr std::size_t DEFAULT_METRIC_HISTORY_SIZE = 120;

/**
 * @brief Named values, counters and timers published by the engine and the game, each with a short history.
 *
 * A metric is created by its first sample. Its kind is set then and does not change.
 * The store keeps the last samples of each metric, oldest first, and can be read from another thread
 * while it is written: the readers get a copy, never a reference into the store.
 *
 * The store has no dependency on the UI, so a server uses it like a client:
 * tools::metrics().increment("network.packets_lost");
 */
class MetricsStore
{
public:
    /**
     * @brief Create a store.
     *
     * @param historySize The number of samples kept for each metric
     *
     * @throw std::invalid_argument if historySize is 0.
     */
    explicit MetricsStore(std::size_t historySize = DEFAULT_METRIC_HISTORY_SIZE);

    MetricsStore(const MetricsStore &) = delete;
    MetricsStore &operator=(const MetricsStore &) = delete;

    /**
     * @brief Add a sample to a metric, creating it as a Value if it does not exist yet.
     *
     * @param name The name of the metric, e.g. "frame_time"
     * @param value The new sample
     * @param kind The kind of the metric, used only when it is created
     *
     * @throw std::invalid_argument if the name is empty.
     */
    void record(std::string_view name, double value, MetricKind kind = MetricKind::Value);

    /**
     * @brief Add an amount to a counter, creating it at 0 if it does not exist yet.
     *        The history keeps the total after each call, so getMetric(name)->last is the current count.
     *
     * @param name The name of the counter, e.g. "packets_lost"
     * @param amount The amount to add, 1 by default
     *
     * @throw std::invalid_argument if the name is empty.
     */
    void increment(std::string_view name, double amount = 1.0);

    /**
     * @brief Get a copy of one metric.
     *
     * @param name The name of the metric
     *
     * @return The metric, or std::nullopt if no sample was recorded under this name.
     */
    [[nodiscard]] std::optional<Metric> getMetric(std::string_view name) const;

    /**
     * @brief Get a copy of every metric, sorted by name.
     *
     * @return The metrics, empty if the store has none.
     */
    [[nodiscard]] std::vector<Metric> getMetrics(void) const;

    /**
     * @brief Get the number of samples kept for each metric.
     *
     * @return The history size given at the creation.
     */
    [[nodiscard]] std::size_t getHistorySize(void) const noexcept;

    /**
     * @brief Remove every metric.
     */
    void clear(void);

private:
    struct Entry
    {
        MetricKind kind;
        std::deque<double> history;
    };

    [[nodiscard]] static Metric makeSnapshot(std::string_view name, MetricKind kind, const std::deque<double> &history);

    /**
     * @brief Add a sample and drop the oldest one when the history is full.
     *
     * @param entry The metric to update, its mutex being held
     * @param value The new sample
     */
    void addSample(Entry &entry, double value);

    std::size_t _historySize;
    mutable std::mutex _mutex;
    std::map<std::string, Entry, std::less<>> _metrics;
};

} // namespace younity::tools
