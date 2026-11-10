/*
** EPITECH PROJECT, 2026
** younity-engine/tests/tools/ScopedTimer
** File description:
** ScopedTimer unit tests
*/

#include <criterion/criterion.h>

#include <optional>
#include <stdexcept>
#include <string>

#include "younity/tools/MetricsStore.hpp"
#include "younity/tools/ScopedTimer.hpp"

using younity::tools::Metric;
using younity::tools::MetricKind;
using younity::tools::MetricsStore;
using younity::tools::ScopedTimer;

Test(tools_scoped_timer, records_the_duration_when_the_scope_ends)
{
    MetricsStore store;

    {
        ScopedTimer timer(store, "physics");
    }

    const std::optional<Metric> metric = store.getMetric("physics");

    cr_assert(metric.has_value());
    cr_assert(metric->kind == MetricKind::Timer);
    cr_assert_eq(metric->history.size(), 1);
    cr_assert(metric->last >= 0.0);
}

Test(tools_scoped_timer, records_the_duration_once_when_stopped)
{
    MetricsStore store;
    ScopedTimer timer(store, "physics");

    timer.stop();
    timer.stop();

    const std::optional<Metric> metric = store.getMetric("physics");

    cr_assert(metric.has_value());
    cr_assert_eq(metric->history.size(), 1);
}

Test(tools_scoped_timer, records_into_the_given_store)
{
    MetricsStore used;
    MetricsStore unused;

    {
        ScopedTimer timer(used, "physics");
    }

    cr_assert(used.getMetric("physics").has_value());
    cr_assert_not(unused.getMetric("physics").has_value());
}

Test(tools_scoped_timer, refuses_an_empty_name)
{
    MetricsStore store;

    cr_assert_throw(ScopedTimer(store, ""), std::invalid_argument);
}
