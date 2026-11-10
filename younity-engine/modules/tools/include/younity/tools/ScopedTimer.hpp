/*
** EPITECH PROJECT, 2026
** younity-engine/modules/tools/include/younity/tools/ScopedTimer
** File description:
** ScopedTimer header file
*/

#pragma once

#include <string>

#include "younity/core/Clock.hpp"
#include "younity/tools/MetricsStore.hpp"

namespace younity::tools {

/**
 * @brief Measure the time spent in a scope and record it as a Timer metric when the scope ends.
 *
 * {
 *     ScopedTimer timer(metrics(), "physics");
 *     scene.update(step);
 * }
 *
 * The duration is in seconds, like younity::Clock. stop() records it earlier; the destructor
 * then does nothing, so the metric is recorded exactly once.
 */
class ScopedTimer
{
public:
    /**
     * @brief Start the timer.
     *
     * @param store The store receiving the duration
     * @param name The name of the timer metric, e.g. "physics"
     *
     * @throw std::invalid_argument if the name is empty.
     */
    ScopedTimer(MetricsStore &store, std::string name);

    /**
     * @brief Record the duration if stop() was not called.
     */
    ~ScopedTimer(void);

    ScopedTimer(const ScopedTimer &) = delete;
    ScopedTimer &operator=(const ScopedTimer &) = delete;

    /**
     * @brief Record the time elapsed since the creation of the timer, once.
     */
    void stop(void);

private:
    MetricsStore &_store;
    std::string _name;
    Clock _clock;
    bool _stopped = false;
};

} // namespace younity::tools
