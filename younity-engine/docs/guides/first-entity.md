# Your first entity and system

This guide builds a missile that flies to the right and disappears after one second. It uses the three ideas of the `ecs` module:

- an **entity** is an id, like a row number;
- a **component** is a piece of data attached to an entity;
- a **system** is the code that runs every frame on the entities having the components it needs.

Link `Younity::Ecs` in your CMake target to follow along.

## 1. Create a registry and an entity

The registry creates the entities and stores their components.

```cpp
namespace ecs = younity::ecs;

ecs::Registry registry;
const ecs::Entity missile = registry.createEntity();
```

## 2. Give it components

`Transform` and `Velocity` come with the engine. The missile starts at the origin and moves 100 pixels per second to the right.

```cpp
ecs::Velocity velocity;

velocity.value = younity::Vector2f{100.0F, 0.0F};
registry.addComponent(missile, ecs::Transform{});
registry.addComponent(missile, velocity);
```

## 3. Add a system of the engine

`MovementSystem` moves each entity by its velocity. The types listed after it are the components an entity needs to be handled by the system.

```cpp
registry.addSystem<ecs::MovementSystem, ecs::Transform, ecs::Velocity>();
```

## 4. Write your own component

A component is a plain struct. This one holds the time the missile has left.

```cpp
struct Lifetime
{
    float seconds;
};
```

## 5. Write your own system

A system inherits from `ecs::System` and receives its entities at each frame. This one counts the time down and destroys the entity at zero.

```cpp
class LifetimeSystem : public ecs::System
{
public:
    void update(ecs::Registry &registry, const std::vector<ecs::Entity> &entities, float dt) override
    {
        for (const ecs::Entity entity : entities) {
            Lifetime &lifetime = registry.getComponent<Lifetime>(entity);

            lifetime.seconds -= dt;
            if (lifetime.seconds <= 0.0F) {
                registry.destroyEntity(entity);
            }
        }
    }
};
```

Register it with the component it needs, and give the missile one second to live:

```cpp
registry.addSystem<LifetimeSystem, Lifetime>();
registry.addComponent(missile, Lifetime{1.0F});
```

## 6. Run the frames

`updateSystems` runs every system once, in the order they were added. Its parameter is the time elapsed since the last frame, in seconds.

```cpp
registry.updateSystems(0.5F);
```

## The whole program

```cpp
#include <iostream>
#include <vector>

#include "younity/ecs/Registry.hpp"
#include "younity/ecs/components/Transform.hpp"
#include "younity/ecs/components/Velocity.hpp"
#include "younity/ecs/systems/MovementSystem.hpp"

namespace ecs = younity::ecs;

struct Lifetime
{
    float seconds;
};

class LifetimeSystem : public ecs::System
{
public:
    void update(ecs::Registry &registry, const std::vector<ecs::Entity> &entities, float dt) override
    {
        for (const ecs::Entity entity : entities) {
            Lifetime &lifetime = registry.getComponent<Lifetime>(entity);

            lifetime.seconds -= dt;
            if (lifetime.seconds <= 0.0F) {
                registry.destroyEntity(entity);
            }
        }
    }
};

int main(void)
{
    ecs::Registry registry;

    registry.addSystem<ecs::MovementSystem, ecs::Transform, ecs::Velocity>();
    registry.addSystem<LifetimeSystem, Lifetime>();

    const ecs::Entity missile = registry.createEntity();
    ecs::Velocity velocity;

    velocity.value = younity::Vector2f{100.0F, 0.0F};
    registry.addComponent(missile, ecs::Transform{});
    registry.addComponent(missile, velocity);
    registry.addComponent(missile, Lifetime{1.0F});

    for (int frame = 1; frame <= 3; ++frame) {
        registry.updateSystems(0.5F);
        if (registry.isAlive(missile)) {
            std::cout << "frame " << frame << ": x = " << registry.getComponent<ecs::Transform>(missile).position.x << '\n';
        } else {
            std::cout << "frame " << frame << ": the missile is gone\n";
        }
    }
    return 0;
}
```

It prints:

```text
frame 1: x = 50
frame 2: the missile is gone
frame 3: the missile is gone
```

## What happened

- At frame 1, the missile moves 50 pixels and has half a second left.
- At frame 2, its time reaches zero and `LifetimeSystem` destroys it.
- An entity destroyed during `updateSystems` stays alive until every system has run, then it is removed with all its components. A system can destroy entities freely while it loops over them.

## Go further

- A game groups its entities and systems in scenes: see `younity::ecs::Scene` and `SceneManager` in the [API reference](../api/annotated.md).
- Systems talk to each other through events: see `younity::EventBus`.
