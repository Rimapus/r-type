/*
** EPITECH PROJECT, 2026
** younity-engine/tests/editor/MetricsWindow
** File description:
** MetricsWindow unit tests
*/

#include <criterion/criterion.h>
#include <imgui.h>

#include "../TestSupport.hpp"
#include "younity/editor/MetricsWindow.hpp"
#include "younity/tools/Metric.hpp"
#include "younity/tools/MetricsStore.hpp"

using younity::Rectf;
using younity::editor::METRICS_TITLE;
using younity::editor::MetricsWindow;
using younity::test::createContext;
using younity::test::destroyContext;
using younity::test::drawFrame;
using younity::test::isDrawn;
using younity::test::isFixedAt;
using younity::tools::MetricKind;
using younity::tools::MetricsStore;

Test(editor_metrics_window, draws_the_metrics_window, .init = createContext, .fini = destroyContext)
{
    MetricsStore store;
    MetricsWindow window(store);

    store.record("frame_time", 0.016, MetricKind::Timer);
    store.record("fps", 60.0);
    store.record("system.MovementSystem", 0.0002, MetricKind::Timer);
    store.increment("network.packets_lost", 3.0);
    drawFrame(window);
    cr_assert(isDrawn(METRICS_TITLE));
}

Test(editor_metrics_window, draws_before_any_metric, .init = createContext, .fini = destroyContext)
{
    MetricsStore store;
    MetricsWindow window(store);

    drawFrame(window);
    cr_assert(isDrawn(METRICS_TITLE));
}

Test(editor_metrics_window, is_fixed_in_its_area, .init = createContext, .fini = destroyContext)
{
    MetricsStore store;
    MetricsWindow window(store);

    ImGui::NewFrame();
    window.draw(Rectf{100.0F, 50.0F, 400.0F, 200.0F});
    ImGui::Render();
    cr_assert(isFixedAt(METRICS_TITLE, Rectf{100.0F, 50.0F, 400.0F, 200.0F}));
}

Test(editor_metrics_window, is_absent_from_a_frame_that_skips_it, .init = createContext, .fini = destroyContext)
{
    MetricsStore store;
    MetricsWindow window(store);

    drawFrame(window);
    ImGui::NewFrame();
    ImGui::Render();
    cr_assert_not(isDrawn(METRICS_TITLE));
}
