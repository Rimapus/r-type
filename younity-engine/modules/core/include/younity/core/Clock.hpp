/*
** EPITECH PROJECT, 2026
** younity-engine/modules/core/include/younity/core/Clock
** File description:
** Clock header file
*/

#pragma once

#include <chrono>
#include <concepts>

namespace younity {

/**
 * @brief A source of time for a clock: a std::chrono clock that never goes back, e.g. std::chrono::steady_clock.
 *
 * The wall clock (std::chrono::system_clock) is refused: it jumps when the date of the computer changes.
 */
template <typename T>
concept TimeSource = T::is_steady && requires {
    // Written out: libc++ (Apple Clang) has no std::chrono::is_clock_v yet
    typename T::duration;
    typename T::time_point;
    { T::now() } -> std::same_as<typename T::time_point>;
};

/**
 * @brief Measures the time elapsed since its creation or its last restart, e.g. the delta time of each frame:
 *        float dt = clock.restart();
 *
 * @tparam Source The source of time, a fake one can be given to the tests
 */
template <TimeSource Source>
class BasicClock
{
public:
    /**
     * @brief Start the clock.
     */
    BasicClock(void) noexcept
        : _start(Source::now())
    {
    }

    /**
     * @brief Get the time elapsed since the creation or the last restart, without restarting.
     *
     * @return The elapsed time, in seconds.
     */
    [[nodiscard]] float getElapsedTime(void) const noexcept
    {
        return std::chrono::duration<float>(Source::now() - this->_start).count();
    }

    /**
     * @brief Restart the clock from 0.
     *        Called once per frame, it gives the delta time: the time the last frame took.
     *
     * @return The time elapsed before the restart, in seconds.
     */
    float restart(void) noexcept
    {
        const typename Source::time_point now = Source::now();
        const float elapsed = std::chrono::duration<float>(now - this->_start).count();

        // A single now() call: no time is lost between the measure and the restart
        this->_start = now;
        return elapsed;
    }

private:
    typename Source::time_point _start;
};

/**
 * @brief The clock of the engine, on the monotonic clock of the system.
 */
using Clock = BasicClock<std::chrono::steady_clock>;

} // namespace younity
