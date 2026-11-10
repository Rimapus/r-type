/*
** EPITECH PROJECT, 2026
** younity-engine/modules/editor/include/younity/editor/MetricsWindow
** File description:
** MetricsWindow header file
*/

#pragma once

#include <optional>

#include "younity/core/Rect.hpp"

namespace younity::tools {
class MetricsStore;
}

namespace younity::editor {

/**
 * @brief Title of the metrics overlay, also its id in the debug interface.
 */
inline constexpr const char *METRICS_TITLE = "Metrics";

/**
 * @brief The metrics overlay drawn over the game: every metric of a store, sorted by name, with its
 *        current value, its minimum, its average and its maximum on the kept history.
 *
 * The engine publishes "fps" and "frame_time", a system profiler written by the game publishes
 * "system.<name>", and the network publishes "network.latency" and "network.packets_lost". A timer
 * is shown in milliseconds. The store is read once per frame, so the overlay can show the metrics
 * published by another thread, like the network one.
 */
class MetricsWindow
{
public:
    /**
     * @brief Create an overlay reading the metrics store shared by the engine and the game.
     */
    MetricsWindow(void);

    /**
     * @brief Create an overlay reading another metrics store, e.g. one owned by a test.
     *
     * @param store The store to read, which must outlive the window
     */
    explicit MetricsWindow(tools::MetricsStore &store) noexcept;

    MetricsWindow(const MetricsWindow &) = delete;
    MetricsWindow &operator=(const MetricsWindow &) = delete;

    /**
     * @brief Draw the metrics window. Called once per frame between window.clear() and window.display().
     *
     * @param area Where the window is fixed, in pixels from the top left corner of the window, e.g. by a Workspace, or
     *        nothing to let the mouse move and resize it
     */
    void draw(const std::optional<Rectf> &area = std::nullopt);

private:
    tools::MetricsStore &_store;
};

} // namespace younity::editor
