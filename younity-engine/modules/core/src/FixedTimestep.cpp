/*
** EPITECH PROJECT, 2026
** younity-engine/modules/core/src/FixedTimestep
** File description:
** FixedTimestep source file
*/

#include "younity/core/FixedTimestep.hpp"
#include "younity/core/Log.hpp"
#include <cmath>
#include <stdexcept>

namespace younity {

FixedTimestep::FixedTimestep(float step, float maxFrameTime)
    : _step(step), _maxFrameTime(maxFrameTime)
{
    if (!std::isfinite(step) || step <= 0.0F) {
        throw std::invalid_argument("younity::FixedTimestep: the step must be a positive number");
    }
    // NaN fails this check too
    if (!(maxFrameTime >= step)) {
        throw std::invalid_argument("younity::FixedTimestep: the max frame time must not be shorter than the step");
    }
}

std::size_t FixedTimestep::advance(float dt)
{
    std::size_t steps = 0;

    // NaN fails this check too
    if (!(dt > 0.0F)) {
        return 0;
    }
    if (dt > this->_maxFrameTime) {
        core::logger().warn("Frame took {}s, only {}s are simulated", dt, this->_maxFrameTime);
        dt = this->_maxFrameTime;
    }
    this->_accumulator += dt;
    while (this->_accumulator >= this->_step) {
        this->_accumulator -= this->_step;
        ++steps;
    }
    return steps;
}

float FixedTimestep::getStep(void) const noexcept
{
    return this->_step;
}

float FixedTimestep::getAlpha(void) const noexcept
{
    return static_cast<float>(this->_accumulator / this->_step);
}

void FixedTimestep::reset(void) noexcept
{
    this->_accumulator = 0.0;
}

} // namespace younity
