/*
** EPITECH PROJECT, 2026
** younity-engine/tests/render/WindowMetrics
** File description:
** Window metrics unit tests
*/

#include <criterion/criterion.h>

#include <chrono>
#include <optional>
#include <thread>

#include "../TestSupport.hpp"
#include "younity/render/Window.hpp"
#include "younity/tools/Global.hpp"
#include "younity/tools/Metric.hpp"

using younity::render::Window;
using younity::test::skipWithoutDisplay;
using younity::tools::Metric;
using younity::tools::MetricKind;

Test(render_window_metrics, clear_publishes_the_frame_time_and_the_frame_rate)
{
    skipWithoutDisplay();
    younity::tools::metrics().clear();

    {
        Window window(320, 240, "Window metrics");

        // clear() records no frame when the clock has not ticked yet
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
        window.clear();
        window.display();
    }
    const std::optional<Metric> frameTime = younity::tools::metrics().getMetric("frame_time");
    const std::optional<Metric> fps = younity::tools::metrics().getMetric("fps");

    cr_assert(frameTime.has_value());
    cr_assert(frameTime->kind == MetricKind::Timer);
    cr_assert_gt(frameTime->last, 0.0);
    cr_assert(fps.has_value());
    cr_assert(fps->kind == MetricKind::Value);
    cr_assert_float_eq(fps->last, 1.0 / frameTime->last, 0.000001);
}

Test(render_window_metrics, a_second_clear_before_display_keeps_the_frame)
{
    skipWithoutDisplay();
    younity::tools::metrics().clear();

    {
        Window window(320, 240, "Window metrics");

        // clear() records no frame when the clock has not ticked yet
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
        window.clear();
        window.clear();
    }
    const std::optional<Metric> frameTime = younity::tools::metrics().getMetric("frame_time");

    cr_assert(frameTime.has_value());
    cr_assert_eq(frameTime->history.size(), 1U);
}
