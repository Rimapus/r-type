/*
** EPITECH PROJECT, 2026
** younity-engine/tests/render/AnimationSystem
** File description:
** AnimationSystem unit tests
*/

#include <criterion/criterion.h>

#include <limits>
#include <utility>
#include <vector>

#include "younity/core/EventBus.hpp"
#include "younity/ecs/Registry.hpp"
#include "younity/render/AnimationClip.hpp"
#include "younity/render/AnimationFinishedEvent.hpp"
#include "younity/render/components/Animation.hpp"
#include "younity/render/systems/AnimationSystem.hpp"

using younity::EventBus;
using younity::Recti;
using younity::ecs::Entity;
using younity::ecs::Registry;
using younity::render::Animation;
using younity::render::AnimationClip;
using younity::render::AnimationFinishedEvent;
using younity::render::AnimationSystem;

namespace {

// A registry with an AnimationSystem, keeping the ends of animation it publishes
struct World
{
    World(void)
    {
        this->bus.subscribe<AnimationFinishedEvent>([this](const AnimationFinishedEvent &event) {
            this->finished.push_back(event.entity);
        });
        this->registry.addSystem<AnimationSystem, Animation>(this->bus);
    }

    EventBus bus;
    Registry registry;
    std::vector<Entity> finished;
};

// Three frames of 10 by 10, a tenth of a second each
AnimationClip threeFrames(bool loop)
{
    return AnimationClip{
        {Recti{0, 0, 10, 10}, Recti{10, 0, 10, 10}, Recti{20, 0, 10, 10}},
        0.1F,
        loop
    };
}

Entity createAnimated(Registry &registry, AnimationClip clip)
{
    const Entity entity = registry.createEntity();

    registry.addComponent(entity, Animation{.clip = std::move(clip)});
    return entity;
}

} // namespace

Test(render_animation_system, starts_at_the_first_frame)
{
    const Animation animation{.clip = threeFrames(true)};

    cr_assert(animation.getCurrentFrame() == (Recti{0, 0, 10, 10}));
}

Test(render_animation_system, short_step_keeps_the_frame)
{
    World world;
    const Entity entity = createAnimated(world.registry, threeFrames(true));

    world.registry.updateSystems(0.05F);
    cr_assert_eq(world.registry.getComponent<Animation>(entity).frame, 0U);
}

Test(render_animation_system, frame_changes_after_its_duration)
{
    World world;
    const Entity entity = createAnimated(world.registry, threeFrames(true));

    world.registry.updateSystems(0.06F);
    world.registry.updateSystems(0.06F);
    const Animation &animation = world.registry.getComponent<Animation>(entity);
    cr_assert_eq(animation.frame, 1U);
    cr_assert(animation.getCurrentFrame() == (Recti{10, 0, 10, 10}));
}

Test(render_animation_system, long_step_skips_frames)
{
    World world;
    const Entity entity = createAnimated(world.registry, threeFrames(true));

    world.registry.updateSystems(0.25F);
    cr_assert_eq(world.registry.getComponent<Animation>(entity).frame, 2U);
}

Test(render_animation_system, looping_clip_goes_back_to_the_first_frame)
{
    World world;
    const Entity entity = createAnimated(world.registry, threeFrames(true));

    for (int step = 0; step < 3; ++step) {
        world.registry.updateSystems(0.1F);
    }
    cr_assert_eq(world.registry.getComponent<Animation>(entity).frame, 0U);
    cr_assert_not(world.registry.getComponent<Animation>(entity).finished);
    cr_assert(world.finished.empty());
}

Test(render_animation_system, clip_played_once_stays_on_its_last_frame)
{
    World world;
    const Entity entity = createAnimated(world.registry, threeFrames(false));

    for (int step = 0; step < 5; ++step) {
        world.registry.updateSystems(0.1F);
    }
    const Animation &animation = world.registry.getComponent<Animation>(entity);
    cr_assert_eq(animation.frame, 2U);
    cr_assert(animation.finished);
}

Test(render_animation_system, end_of_a_clip_played_once_is_published_once)
{
    World world;
    const Entity entity = createAnimated(world.registry, threeFrames(false));

    for (int step = 0; step < 5; ++step) {
        world.registry.updateSystems(0.1F);
    }
    cr_assert_eq(world.finished.size(), 1U);
    cr_assert_eq(world.finished[0], entity);
}

Test(render_animation_system, end_is_published_after_the_last_frame_was_shown)
{
    World world;
    (void)createAnimated(world.registry, threeFrames(false));

    // 0.2 s: the last frame starts, it still has to be shown for its duration
    world.registry.updateSystems(0.2F);
    cr_assert(world.finished.empty());
    world.registry.updateSystems(0.1F);
    cr_assert_eq(world.finished.size(), 1U);
}

Test(render_animation_system, play_restarts_with_another_clip)
{
    World world;
    const Entity entity = createAnimated(world.registry, threeFrames(false));

    for (int step = 0; step < 5; ++step) {
        world.registry.updateSystems(0.1F);
    }
    Animation &animation = world.registry.getComponent<Animation>(entity);
    animation.play(threeFrames(true));
    cr_assert_eq(animation.frame, 0U);
    cr_assert_float_eq(animation.elapsed, 0.0F, 0.00001F);
    cr_assert_not(animation.finished);
    world.registry.updateSystems(0.1F);
    cr_assert_eq(world.registry.getComponent<Animation>(entity).frame, 1U);
}

Test(render_animation_system, empty_clip_has_no_frame_and_does_not_move)
{
    World world;
    const Entity entity = createAnimated(world.registry, AnimationClip{});

    world.registry.updateSystems(1.0F);
    const Animation &animation = world.registry.getComponent<Animation>(entity);
    cr_assert_not(animation.getCurrentFrame().has_value());
    cr_assert(world.finished.empty());
}

Test(render_animation_system, frame_out_of_the_clip_has_no_frame)
{
    Animation animation{.clip = threeFrames(true)};

    animation.frame = 7;
    cr_assert_not(animation.getCurrentFrame().has_value());
}

Test(render_animation_system, entities_are_animated_independently)
{
    World world;
    const Entity first = createAnimated(world.registry, threeFrames(true));

    world.registry.updateSystems(0.1F);
    const Entity second = createAnimated(world.registry, threeFrames(true));
    world.registry.updateSystems(0.1F);
    cr_assert_eq(world.registry.getComponent<Animation>(first).frame, 2U);
    cr_assert_eq(world.registry.getComponent<Animation>(second).frame, 1U);
}

Test(render_animation_system, huge_step_does_not_hang, .timeout = 2)
{
    World world;
    const Entity looping = createAnimated(world.registry, threeFrames(true));
    const Entity once = createAnimated(world.registry, threeFrames(false));

    world.registry.updateSystems(1.0e30F);
    cr_assert_lt(world.registry.getComponent<Animation>(looping).frame, 3U);
    cr_assert(world.registry.getComponent<Animation>(once).finished);
}

Test(render_animation_system, tiny_frame_duration_does_not_hang, .timeout = 2)
{
    World world;
    const Entity entity = createAnimated(world.registry, AnimationClip{
                                                             {Recti{0, 0, 1, 1}, Recti{1, 0, 1, 1}},
                                                             1.0e-30F,
                                                             true
    });

    world.registry.updateSystems(1.0F);
    cr_assert_lt(world.registry.getComponent<Animation>(entity).frame, 2U);
}

Test(render_animation_system, infinite_or_nan_step_is_ignored)
{
    World world;
    const Entity entity = createAnimated(world.registry, threeFrames(true));

    world.registry.updateSystems(std::numeric_limits<float>::infinity());
    world.registry.updateSystems(std::numeric_limits<float>::quiet_NaN());
    const Animation &animation = world.registry.getComponent<Animation>(entity);
    cr_assert_eq(animation.frame, 0U);
    cr_assert_float_eq(animation.elapsed, 0.0F, 0.00001F);
}

Test(render_animation_system, looping_clip_wraps_after_several_rounds)
{
    World world;
    const Entity entity = createAnimated(world.registry, threeFrames(true));

    // 7 frames of time: two full rounds, then one more frame
    world.registry.updateSystems(0.75F);
    cr_assert_eq(world.registry.getComponent<Animation>(entity).frame, 1U);
}

Test(render_animation_system, frame_set_out_of_the_clip_starts_again)
{
    World world;
    const Entity looping = createAnimated(world.registry, threeFrames(true));
    const Entity once = createAnimated(world.registry, threeFrames(false));

    world.registry.getComponent<Animation>(looping).frame = 7;
    world.registry.getComponent<Animation>(once).frame = 7;
    world.registry.updateSystems(0.05F);
    cr_assert_eq(world.registry.getComponent<Animation>(looping).frame, 0U);
    cr_assert_eq(world.registry.getComponent<Animation>(once).frame, 2U);
}
