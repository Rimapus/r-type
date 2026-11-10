/*
** EPITECH PROJECT, 2026
** younity-engine/modules/core/include/younity/core/FixedTimestep
** File description:
** FixedTimestep header file
*/

#pragma once

#include <cstddef>

namespace younity {

/**
 * @brief Turns the variable time of the frames into a number of steps of a fixed duration,
 *        so the physics and the network run at the same rate whatever the frame rate is:
 *
 *        const float dt = clock.restart();
 *        for (std::size_t steps = timestep.advance(dt); steps > 0; --steps) {
 *            registry.updateSystems(timestep.getStep());
 *        }
 *        render(timestep.getAlpha());
 *
 * The time left after the steps is kept for the next frame, so no time is lost.
 */
class FixedTimestep
{
public:
    /**
     * @brief The longest frame time taken into account by default, in seconds.
     */
    static constexpr float DEFAULT_MAX_FRAME_TIME = 0.25F;

    /**
     * @brief Create a timestep with no time accumulated yet.
     *
     * @param step The duration of one step, in seconds, e.g. 1.0F / 60.0F for 60 steps per second
     * @param maxFrameTime The longest frame time taken into account, in seconds: a longer frame, e.g. after
     *                     a breakpoint, is shortened to it, instead of running a lot of steps to catch up
     *                     and making the next frame even longer
     *
     * @throw std::invalid_argument if the step is not a positive number, or if maxFrameTime is shorter than it.
     */
    explicit FixedTimestep(float step, float maxFrameTime = DEFAULT_MAX_FRAME_TIME);

    /**
     * @brief Add the time of a frame, and remove the time of the steps to run from it.
     *
     * @param dt The time the frame took, in seconds (delta time). A negative or NaN time is ignored.
     *
     * @return The number of steps to run this frame, possibly 0.
     */
    [[nodiscard]] std::size_t advance(float dt);

    /**
     * @brief Get the duration of one step, the delta time to give to each step.
     *
     * @return The step given at the creation, in seconds.
     */
    [[nodiscard]] float getStep(void) const noexcept;

    /**
     * @brief Get how far the time is between the last step and the next one,
     *        to draw the entities between their last two states.
     *
     * @return The time left divided by the step, between 0 included and 1 excluded.
     */
    [[nodiscard]] float getAlpha(void) const noexcept;

    /**
     * @brief Drop the time left, e.g. after a loading screen.
     */
    void reset(void) noexcept;

private:
    float _step;
    float _maxFrameTime;
    // In double: adding small frame times to it for hours must not lose precision
    double _accumulator = 0.0;
};

} // namespace younity
