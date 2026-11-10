/*
** EPITECH PROJECT, 2026
** younity-engine/tests/ecs/SceneManager
** File description:
** SceneManager unit tests
*/

#include <criterion/criterion.h>

#include <stdexcept>
#include <vector>

#include "younity/ecs/SceneManager.hpp"

using younity::ecs::Entity;
using younity::ecs::Registry;
using younity::ecs::Scene;
using younity::ecs::SceneManager;
using younity::ecs::System;

namespace {

// What happened to a scene, read by the tests
struct SceneLog
{
    int updates = 0;
    bool destroyed = false;
};

// Writes in its log when it is updated and when it is destroyed
class RecordScene : public Scene
{
public:
    explicit RecordScene(SceneLog &log)
        : _log(log) {}

    ~RecordScene(void) override
    {
        this->_log.destroyed = true;
    }

    void update(float dt) override
    {
        Scene::update(dt);
        ++this->_log.updates;
    }

private:
    SceneLog &_log;
};

// Asks to be replaced by a RecordScene during its own update
class SwitchScene : public Scene
{
public:
    SwitchScene(SceneManager &scenes, SceneLog &nextLog)
        : _scenes(scenes), _nextLog(nextLog) {}

    void update(float dt) override
    {
        Scene::update(dt);
        this->_scenes.changeScene<RecordScene>(this->_nextLog);
    }

private:
    SceneManager &_scenes;
    SceneLog &_nextLog;
};

// Counts how many times it runs
class CountSystem : public System
{
public:
    explicit CountSystem(int &count)
        : _count(count) {}

    void update(Registry &, const std::vector<Entity> &, float) override
    {
        ++this->_count;
    }

private:
    int &_count;
};

// A scene with one system, added in its constructor like in a game
class SystemScene : public Scene
{
public:
    explicit SystemScene(int &count)
    {
        this->getRegistry().addSystem<CountSystem>(count);
    }
};

} // namespace

Test(ecs_scene_manager, update_without_scene_does_nothing)
{
    SceneManager scenes;

    cr_assert_no_throw(scenes.update(1.0F), std::exception);
    cr_assert_not(scenes.hasActiveScene());
}

Test(ecs_scene_manager, get_the_active_scene_without_scene_throws)
{
    SceneManager scenes;

    cr_assert_throw((void)scenes.getActiveScene(), std::logic_error);
}

Test(ecs_scene_manager, scene_becomes_active_at_the_next_update)
{
    SceneLog log;
    SceneManager scenes;

    scenes.changeScene<RecordScene>(log);
    cr_assert_not(scenes.hasActiveScene());

    scenes.update(1.0F);
    cr_assert(scenes.hasActiveScene());
}

Test(ecs_scene_manager, active_scene_is_updated_each_frame)
{
    SceneLog log;
    SceneManager scenes;

    scenes.changeScene<RecordScene>(log);
    scenes.update(1.0F);
    scenes.update(1.0F);
    cr_assert_eq(log.updates, 2);
}

Test(ecs_scene_manager, changing_scene_destroys_the_previous_one_at_the_next_update)
{
    SceneLog firstLog;
    SceneLog secondLog;
    SceneManager scenes;

    scenes.changeScene<RecordScene>(firstLog);
    scenes.update(1.0F);
    scenes.changeScene<RecordScene>(secondLog);
    cr_assert_not(firstLog.destroyed);

    scenes.update(1.0F);
    cr_assert(firstLog.destroyed);
    cr_assert_eq(firstLog.updates, 1);
    cr_assert_eq(secondLog.updates, 1);
}

Test(ecs_scene_manager, scene_can_change_scene_during_its_update)
{
    SceneLog nextLog;
    SceneManager scenes;

    scenes.changeScene<SwitchScene>(scenes, nextLog);
    scenes.update(1.0F);
    cr_assert_eq(nextLog.updates, 0);

    scenes.update(1.0F);
    cr_assert_eq(nextLog.updates, 1);
}

Test(ecs_scene_manager, scene_update_runs_the_systems_of_its_registry)
{
    int count = 0;
    SceneManager scenes;

    scenes.changeScene<SystemScene>(count);
    scenes.update(1.0F);
    scenes.update(1.0F);
    cr_assert_eq(count, 2);
}

Test(ecs_scene_manager, active_scene_gives_its_registry_and_its_event_bus)
{
    SceneLog log;
    int received = 0;
    SceneManager scenes;

    scenes.changeScene<RecordScene>(log);
    scenes.update(1.0F);
    Scene &scene = scenes.getActiveScene();
    const Entity entity = scene.getRegistry().createEntity();

    scene.getEventBus().subscribe<int>([&received](const int &value) {
        received = value;
    });
    scene.getEventBus().publish(42);
    cr_assert(scene.getRegistry().isAlive(entity));
    cr_assert_eq(received, 42);
}
