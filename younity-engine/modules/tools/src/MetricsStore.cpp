/*
** EPITECH PROJECT, 2026
** younity-engine/modules/tools/src/MetricsStore
** File description:
** MetricsStore source file
*/

#include "younity/tools/MetricsStore.hpp"
#include <algorithm>
#include <numeric>
#include <stdexcept>
#include <utility>

namespace younity::tools {

MetricsStore::MetricsStore(std::size_t historySize)
    : _historySize(historySize)
{
    if (historySize == 0) {
        throw std::invalid_argument("younity::tools::MetricsStore: the history size must be positive");
    }
}

void MetricsStore::record(std::string_view name, double value, MetricKind kind)
{
    if (name.empty()) {
        throw std::invalid_argument("younity::tools::MetricsStore: the name of a metric cannot be empty");
    }
    const std::lock_guard lock(this->_mutex);
    const auto it = this->_metrics.find(name);

    if (it == this->_metrics.end()) {
        Entry &entry = this->_metrics.emplace(std::string(name), Entry{.kind = kind, .history = {}}).first->second;

        this->addSample(entry, value);
        return;
    }
    this->addSample(it->second, value);
}

void MetricsStore::increment(std::string_view name, double amount)
{
    if (name.empty()) {
        throw std::invalid_argument("younity::tools::MetricsStore: the name of a counter cannot be empty");
    }
    const std::lock_guard lock(this->_mutex);
    const auto it = this->_metrics.find(name);

    if (it == this->_metrics.end()) {
        Entry &entry = this->_metrics.emplace(std::string(name), Entry{.kind = MetricKind::Counter, .history = {}}).first->second;

        this->addSample(entry, amount);
        return;
    }
    const double value = it->second.history.empty() ? amount : it->second.history.back() + amount;

    this->addSample(it->second, value);
}

std::optional<Metric> MetricsStore::getMetric(std::string_view name) const
{
    const std::lock_guard lock(this->_mutex);
    const auto it = this->_metrics.find(name);

    if (it == this->_metrics.end()) {
        return std::nullopt;
    }
    return makeSnapshot(it->first, it->second.kind, it->second.history);
}

std::vector<Metric> MetricsStore::getMetrics(void) const
{
    const std::lock_guard lock(this->_mutex);
    std::vector<Metric> metrics;

    metrics.reserve(this->_metrics.size());
    for (const auto &[name, entry] : this->_metrics) {
        metrics.push_back(makeSnapshot(name, entry.kind, entry.history));
    }
    return metrics;
}

std::size_t MetricsStore::getHistorySize(void) const noexcept
{
    return this->_historySize;
}

void MetricsStore::clear(void)
{
    const std::lock_guard lock(this->_mutex);

    this->_metrics.clear();
}

Metric MetricsStore::makeSnapshot(std::string_view name, MetricKind kind, const std::deque<double> &history)
{
    Metric metric;

    metric.name = std::string(name);
    metric.kind = kind;
    metric.history.assign(history.begin(), history.end());

    const auto [minimum, maximum] = std::minmax_element(metric.history.begin(), metric.history.end());
    metric.last = metric.history.back();
    metric.minimum = *minimum;
    metric.maximum = *maximum;
    metric.average = std::accumulate(metric.history.begin(), metric.history.end(), 0.0) / static_cast<double>(metric.history.size());
    return metric;
}

void MetricsStore::addSample(Entry &entry, double value)
{
    entry.history.push_back(value);
    if (entry.history.size() > this->_historySize) {
        entry.history.pop_front();
    }
}

} // namespace younity::tools
