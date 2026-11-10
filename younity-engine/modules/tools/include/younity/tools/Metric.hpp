/*
** EPITECH PROJECT, 2026
** younity-engine/modules/tools/include/younity/tools/Metric
** File description:
** Metric header file
*/

#pragma once

#include <string>
#include <vector>

namespace younity::tools {

/**
 * @brief What a metric measures, so a reader can format it.
 */
enum class MetricKind {
    /** @brief A value that can go up and down, e.g. the frame time. */
    Value,
    /** @brief A total that only goes up, e.g. the packets lost. */
    Counter,
    /** @brief A duration in seconds, e.g. the time spent in a system. */
    Timer,
};

/**
 * @brief A copy of one metric of the store: its name, its kind, its recent history and its statistics.
 *
 * The history goes from the oldest sample to the newest, and always holds at least one sample.
 */
struct Metric
{
    /** @brief The name of the metric, e.g. "network.packets_lost". */
    std::string name;
    /** @brief What the metric measures. */
    MetricKind kind = MetricKind::Value;
    /** @brief The kept samples, from the oldest to the newest. */
    std::vector<double> history;
    /** @brief The value of the newest sample. */
    double last = 0.0;
    /** @brief The smallest sample of the history. */
    double minimum = 0.0;
    /** @brief The largest sample of the history. */
    double maximum = 0.0;
    /** @brief The average of the history. */
    double average = 0.0;
};

} // namespace younity::tools
