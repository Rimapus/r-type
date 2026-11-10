/*
** EPITECH PROJECT, 2026
** younity-engine/modules/render/src/systems/AnimationSystem
** File description:
** AnimationSystem source file
*/

#include "younity/render/systems/AnimationSystem.hpp"
#include "younity/ecs/Registry.hpp"
#include "younity/render/AnimationFinishedEvent.hpp"
#include "younity/render/components/Animation.hpp"
#include <cmath>
#include <cstddef>

namespace younity::render {

namespace {

/**
 * @brief Move an animation forward by the time of a step.
 *        The frames to skip are computed at once, not one by one: a huge step or a tiny frame duration
 *        cannot make it loop for a long time.
 *
 * @param animation The animation to move forward
 * @param dt The time of the step, in seconds
 *
 * @return true if a clip that does not loop has just ended.
 */
bool advance(Animation &animation, float dt)
{
    const AnimationClip &clip = animation.clip;
    const std::size_t count = clip.frames.size();

    // NaN fails these checks too
    if (animation.finished || count == 0 || !(clip.frameDuration > 0.0F) || !std::isfinite(dt) || !(dt > 0.0F)) {
        return false;
    }
    // A frame set out of the clip by the game: back to the first one, or to the last one if the clip does not loop
    if (animation.frame >= count) {
        animation.frame = clip.loop ? 0 : count - 1;
    }

    const double elapsed = static_cast<double>(animation.elapsed) + dt;
    const double duration = clip.frameDuration;
    const double steps = std::floor(elapsed / duration);

    animation.elapsed = static_cast<float>(std::fmod(elapsed, duration));
    if (clip.loop) {
        animation.frame = (animation.frame + static_cast<std::size_t>(std::fmod(steps, static_cast<double>(count)))) % count;
        return false;
    }
    // The last frame is shown for its whole duration before the clip ends
    if (steps <= static_cast<double>(count - 1 - animation.frame)) {
        animation.frame += static_cast<std::size_t>(steps);
        return false;
    }
    animation.frame = count - 1;
    animation.elapsed = 0.0F;
    animation.finished = true;
    return true;
}

} // namespace

AnimationSystem::AnimationSystem(EventBus &bus)
    : _bus(bus) {}

void AnimationSystem::update(ecs::Registry &registry, const std::vector<ecs::Entity> &entities, float dt)
{
    for (const ecs::Entity entity : entities) {
        if (advance(registry.getComponent<Animation>(entity), dt)) {
            this->_bus.publish(AnimationFinishedEvent{entity});
        }
    }
}

} // namespace younity::render
