# Physics

The `physics` module makes entities fall, stops them on walls and the ground, and tells when two entities touch. It detects the overlap and announces it; what a collision means belongs to the game.

The headers are in `modules/physics/include/younity/physics/`, in the namespace `younity::physics`. The module is built on `core` and `ecs`.

## Hitboxes

A `Hitbox` is a rectangle attached to an entity: a `size`, and an `offset` from the position of the entity. The position itself comes from the `Transform`.

```text
(100, 200)  Transform.position
   ┌──────────────────────────┐
   │ sprite, 32 x 16          │
   │   ┌────────────────┐     │
   │   │ Hitbox, 24 x 8 │     │  offset (4, 4)
   │   └────────────────┘     │
   └──────────────────────────┘
```

The hitbox is often smaller than the picture of the entity, as in most shoot'em ups. The scale of the `Transform` resizes the hitbox with the entity. Rectangles stay aligned with the axes, so the rotation is left aside.

## The collision system

`CollisionSystem` runs on the entities having a `Transform` and a `Hitbox`. At each frame it computes the rectangle of every entity, compares each pair once, and publishes a `CollisionEvent` for each pair that overlaps.

```cpp
younity::EventBus bus;
ecs::Registry registry;

bus.subscribe<physics::CollisionEvent>([](const physics::CollisionEvent &event) {
    std::cout << event.first.getId() << " touches " << event.second.getId() << '\n';
});
registry.addSystem<physics::CollisionSystem, ecs::Transform, physics::Hitbox>(bus);
```

Two entities with overlapping hitboxes then produce one event per frame, for as long as they overlap. Rectangles that only share an edge are apart.

## Reacting to a collision

The game subscribes to `CollisionEvent` and applies its own rules: remove health, destroy the missile, play a sound, add points. Several parts of the game can subscribe to the same event, each for its own reason.

The subscribers are called while the collision system is still comparing pairs, and they can change the world freely. The system computed all the rectangles before the first comparison, and the entities destroyed by a subscriber stay alive until the end of the frame.

## Gravity

`GravitySystem` receives the gravity of the world, in pixels per second squared, and adds it to the `Velocity` of every entity having a `Gravity`, at each frame. The `scale` of the `Gravity` multiplies it per entity: `0` floats, `2` falls twice as fast, `-1` rises.

```cpp
registry.addSystem<physics::GravitySystem, ecs::Velocity, physics::Gravity>(Vector2f{0.0F, 980.0F});
registry.addSystem<ecs::MovementSystem, ecs::Transform, ecs::Velocity>();
```

The gravity system is added before the movement system, so the entity moves with the speed it gained during the frame.

## Solids

A `Solid` is an obstacle, like a wall or the ground. A `Body` stops on the solids: `SolidCollisionSystem` runs after the movement and pushes each body out of the solids it overlaps, along the axis where it overlaps the least.

| On the pushed axis     | `bounce` | Result                     |
|------------------------|----------|----------------------------|
| Speed into the solid   | `0`      | Stops                      |
| Speed into the solid   | `0.5`    | Sent back at half speed    |
| Speed into the solid   | `1`      | Sent back at full speed    |
| Speed out of the solid | any      | Kept                       |

The speed on the other axis is kept, so a body that falls on the ground while running slides along it. `Body::onGround` is `true` when a solid pushed the body up during the last step; with gravity, a body on the ground sinks a little at each step and is pushed up again, so it stays `true`.

```cpp
registry.addSystem<physics::GravitySystem, ecs::Velocity, physics::Gravity>(Vector2f{0.0F, 980.0F});
registry.addSystem<ecs::MovementSystem, ecs::Transform, ecs::Velocity>();
registry.addSystem<physics::SolidCollisionSystem, ecs::Transform, ecs::Velocity, physics::Hitbox, physics::Body>();
```

A body moves by `velocity * dt` in one step, and one that crosses a whole solid in a single step goes through it: a solid stays thicker than the distance a body travels in a step. Two bodies pass through each other, and the `CollisionEvent` still reports them.

## Main types

| Type                   | Role                                                                                                    |
|------------------------|---------------------------------------------------------------------------------------------------------|
| `Gravity`              | Component: how much the gravity of the world pulls an entity                                            |
| `GravitySystem`        | Adds the gravity of the world to the velocity of the entities                                           |
| `Hitbox`               | Component: the collision rectangle of an entity                                                         |
| `CollisionSystem`      | Detects the overlapping hitboxes and publishes events                                                   |
| `CollisionEvent`       | The two entities that overlap                                                                           |
| `Solid`                | Component: an obstacle the bodies stop on                                                               |
| `Body`                 | Component: the bounce factor of an entity, and whether it stands on the ground                          |
| `SolidCollisionSystem` | Pushes the bodies out of the solids, so they stop, slide and bounce                                     |
| `getBounds`            | The rectangle of a hitbox in the world                                                                  |
| `registerComponents`   | Registers `Body`, `Gravity`, `Hitbox` and `Solid` in the [component registry](ecs.md#component-registry) |
