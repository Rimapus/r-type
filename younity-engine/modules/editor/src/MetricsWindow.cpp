/*
** EPITECH PROJECT, 2026
** younity-engine/modules/editor/src/MetricsWindow
** File description:
** MetricsWindow source file
*/

#include "younity/editor/MetricsWindow.hpp"
#include "younity/render/DebugWindow.hpp"
#include "younity/tools/Global.hpp"
#include "younity/tools/Metric.hpp"
#include "younity/tools/MetricsStore.hpp"
#include <imgui.h>
#include <iomanip>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

namespace younity::editor {

namespace {

/**
 * @brief Format one sample of a metric: a timer in milliseconds, a counter as a whole number, a value with two decimals.
 *
 * @param kind What the metric measures
 * @param value The sample to format
 *
 * @return The text to show in the table.
 */
std::string formatValue(tools::MetricKind kind, double value)
{
    std::ostringstream text;

    switch (kind) {
    case tools::MetricKind::Timer:
        text << std::fixed << std::setprecision(3) << value * 1000.0 << " ms";
        break;
    case tools::MetricKind::Counter:
        text << std::fixed << std::setprecision(0) << value;
        break;
    case tools::MetricKind::Value:
        text << std::fixed << std::setprecision(2) << value;
        break;
    }
    return text.str();
}

/**
 * @brief Get the column label of a metric kind.
 *
 * @param kind What the metric measures
 *
 * @return The label to show in the table.
 */
const char *kindName(tools::MetricKind kind)
{
    switch (kind) {
    case tools::MetricKind::Timer:
        return "timer";
    case tools::MetricKind::Counter:
        return "counter";
    case tools::MetricKind::Value:
        return "value";
    }
    return "value";
}

} // namespace

MetricsWindow::MetricsWindow(void)
    : _store(tools::metrics()) {}

MetricsWindow::MetricsWindow(tools::MetricsStore &store) noexcept
    : _store(store) {}

void MetricsWindow::draw(const std::optional<Rectf> &area)
{
    const std::vector<tools::Metric> metrics = this->_store.getMetrics();

    ImGui::SetNextWindowSize(ImVec2(620.0F, 360.0F), ImGuiCond_FirstUseEver);
    if (!render::beginDebugWindow(METRICS_TITLE, area)) {
        ImGui::End();
        return;
    }
    if (metrics.empty()) {
        ImGui::TextDisabled("No metric recorded yet");
        ImGui::End();
        return;
    }
    if (ImGui::BeginTable(
            "##metrics.table",
            6,
            ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable | ImGuiTableFlags_SizingStretchProp)) {
        ImGui::TableSetupColumn("Metric");
        ImGui::TableSetupColumn("Kind");
        ImGui::TableSetupColumn("Last");
        ImGui::TableSetupColumn("Min");
        ImGui::TableSetupColumn("Average");
        ImGui::TableSetupColumn("Max");
        ImGui::TableHeadersRow();

        for (const tools::Metric &metric : metrics) {
            const std::string last = formatValue(metric.kind, metric.last);
            const std::string minimum = formatValue(metric.kind, metric.minimum);
            const std::string average = formatValue(metric.kind, metric.average);
            const std::string maximum = formatValue(metric.kind, metric.maximum);

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::TextUnformatted(metric.name.c_str());
            ImGui::TableNextColumn();
            ImGui::TextUnformatted(kindName(metric.kind));
            ImGui::TableNextColumn();
            ImGui::TextUnformatted(last.c_str());
            ImGui::TableNextColumn();
            ImGui::TextUnformatted(minimum.c_str());
            ImGui::TableNextColumn();
            ImGui::TextUnformatted(average.c_str());
            ImGui::TableNextColumn();
            ImGui::TextUnformatted(maximum.c_str());
        }
        ImGui::EndTable();
    }
    ImGui::End();
}

} // namespace younity::editor
