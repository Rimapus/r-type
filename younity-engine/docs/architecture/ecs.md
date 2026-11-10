# ECS

The `ecs` module is the heart of the engine. It stores everything that exists in the game and runs the code that makes it move. It follows the entity-component-system pattern:

- an **entity** is an id;
- a **component** is a piece of data attached to an entity;
- a **system** is the code that runs every frame on the entities having the components it needs.

A missile is an entity with a `Transform`, a `Velocity` and a `Hitbox`. This set of components is what makes it a missile: it moves because it has a velocity, and it hits because it has a hitbox.

The headers are in `modules/ecs/include/younity/ecs/`, in the namespace `younity::ecs`. The [first guide](../guides/first-entity.md) builds a small program with them.

## The registry

`Registry` creates the entities, stores their components and runs the systems. Almost everything goes through it.

```cpp
ecs::Registry registry;
const ecs::Entity player = registry.createEntity();

registry.addComponent(player, ecs::Transform{});
registry.addComponent(player, Health{100});

registry.getComponent<Health>(player).value -= 10;
registry.hasComponent<ecs::Velocity>(player);   // false
registry.destroyEntity(player);
```

A component is any struct: the engine brings `Transform` and `Velocity`, and a game adds its own, like `Health` above.

`createEntity(id)` creates an entity with a given id, the way a scene file restores its entities, and `destroyEntities` destroys every entity while the systems stay registered. `clear` removes the systems too.

## How components are stored

Each component type has its own `SparseArray`: an array where the slot `n` holds the component of the entity with the id `n`, or stays empty.

```text
              entity 0   entity 1   entity 2   entity 3
Transform     (0, 0)     (40, 8)    (12, 30)   empty
Velocity      empty      (100, 0)   empty      empty
Health        100        empty      30         empty
```

Reading the component of an entity is a direct access to a slot. The id of a destroyed entity is given to the next one created, so the arrays stay as small as the game needs.

## Systems and signatures

A system inherits from `System` and is added to the registry with the list of components it needs:

```cpp
registry.addSystem<ecs::MovementSystem, ecs::Transform, ecs::Velocity>();
```

The registry gives a number to each component type and keeps a **signature** for every entity: a set of bits, one per component type, set when the entity has it. A system has a signature too, made of the components it asked for.

```text
                    Transform  Velocity  Health
entity 1                1         1        0
MovementSystem          1         1        0     → entity 1 is in
entity 2                1         0        1     → entity 2 stays out
```

Each time a component is added or removed, the registry compares the signature of the entity with the one of each system, and updates the list of entities of the systems. At each frame, a system receives its list ready to use:

```cpp
void update(ecs::Registry &registry, const std::vector<ecs::Entity> &entities, float dt) override
{
    for (const ecs::Entity entity : entities) {
        // every entity here has a Transform and a Velocity
    }
}
```

`registry.updateSystems(dt)` runs the systems one after the other, in the order they were added.

Code that runs outside the systems asks the registry directly. `getEntitiesWith` gives the alive entities that have all the listed components, sorted by id:

```cpp
for (const ecs::Entity entity : registry.getEntitiesWith<ecs::Transform, render::Sprite>()) {
    // ...
}
```

It goes through every entity at each call, where a system gets its list ready. It suits code that runs once per frame, like drawing, and a system stays the better choice for code that runs at every step.

`getEntities` gives all the alive entities, sorted by id, whatever their components.

## Profiling the systems

Every system has a name. `addSystem` gives it the name of its class, without the namespaces: a `MovementSystem` is named `MovementSystem`. `setName` replaces it, and an empty name throws `std::invalid_argument`.

The registry can call a profiler after each system runs, with the name and the duration in seconds:

```cpp
registry.setSystemProfiler([](std::string_view name, double seconds) {
    tools::metrics().record("system." + std::string(name), seconds, tools::MetricKind::Timer);
});
```

The registry has no dependency on `tools`: the callback is where the game decides what to do with the measure, here recording it in the [metrics store](tools.md#metrics) under `system.<name>`. `setSystemProfiler({})` stops the calls. The [metrics overlay](editor.md#metrics-overlay) shows the samples.

## Changing the world during a frame

A system often changes the world while it loops: a collision destroys two entities, a pickup adds a component. Two rules keep this safe.

The list a system receives is a copy made just before its update, so it stays the same during the loop whatever the system does.

An entity destroyed during `updateSystems` stays alive until every system has run. The systems that come later in the frame still find it with all its components, and it is removed at the end.

## Scenes

A `Scene` is a screen of the game, like the menu or a level. It owns a registry and an event bus. Its constructor creates its entities and systems, and everything is released when the scene is destroyed.

```cpp
class GameScene : public ecs::Scene
{
public:
    GameScene(void)
    {
        ecs::Registry &registry = this->getRegistry();
        const ecs::Entity player = registry.createEntity();

        registry.addSystem<ecs::MovementSystem, ecs::Transform, ecs::Velocity>();
        registry.addComponent(player, ecs::Transform{});
        registry.addComponent(player, ecs::Velocity{});
    }
};
```

`SceneManager` holds the active scene and updates it:

```cpp
ecs::SceneManager scenes;

scenes.changeScene<GameScene>();
scenes.update(dt);
```

`changeScene` can be called from a system of the scene that is running. The switch happens at the beginning of the next update, once the running scene has finished its frame.

## Component registry

A `ComponentRegistry` knows the component types by their name, so a scene file, the editor or a script reaches a component without its C++ type. The game creates one and shares it with all its registries. Each module registers its components with `registerComponents`:

```cpp
ecs::ComponentRegistry components;

ecs::registerComponents(components);                 // Transform, Velocity
physics::registerComponents(components);             // Body, Gravity, Hitbox, Solid
render::registerComponents(components, resources);   // Sprite, Text, Animation, with the game's resource manager
components.add<Health>("Health").field("value", &Health::value);
```

The `ComponentInfo` of a name adds, removes, reads and writes the component of an entity:

```cpp
const ecs::ComponentInfo &transform = components.get("Transform");

transform.add(registry, player);
transform.setField(registry, player, "rotation", 90.0F);
transform.getField(registry, player, "position");   // FieldValue holding a Vector2f
```

`getNamesOf` gives the names of the registered components an entity has, sorted. With `getEntities`, an editor lists the whole world:

```cpp
for (const ecs::Entity entity : registry.getEntities()) {
    for (const std::string &name : components.getNamesOf(registry, entity)) {
        // "Transform", "Velocity", ...
    }
}
```

| Field type in the component                 | Read and written as  |
|---------------------------------------------|----------------------|
| `bool`, `float`, `std::string`              | The same type        |
| Any integer, like `std::uint8_t`            | `int`, range-checked |
| `Vector2f`, `Color`, `std::optional<Recti>` | The same type        |
| `std::vector<Recti>`                        | The same type        |

`getFieldType` gives the type of a field, which a scene loader uses to check each JSON value before writing it.

A value the component does not hold as is gets a field read and written through two functions, with the types of the component and of the field given explicitly. The setter refuses a value by throwing `std::invalid_argument`, and the field keeps the value it had:

```cpp
components.add<Health>("Health").field<Health, int>(
    "percent",
    [](const Health &health) { return health.value / 2; },
    [](Health &health, int percent) { health.value = percent * 2; });
```

The texture of a `Sprite` and the font of a `Text` are resources: their fields hold the path the resource manager loaded them from, so a scene file saves a sprite with its texture and a load reads it again through the same manager. An `Animation` registers the frames, duration and loop of its clip, so a scene file keeps them. Each new component is registered in the `registerComponents` of its module, with every field of a `FieldValue` type.

## Scene files

`saveScene` and `loadScene`, in `SceneFile.hpp`, write and read a scene as JSON. They take the `ComponentRegistry`, so a component is reached by its name without its C++ type:

```cpp
const std::expected<void, ecs::SceneFileError> saved = ecs::saveScene(registry, components, "level.json");
const std::expected<void, ecs::SceneFileError> loaded = ecs::loadScene(registry, components, "level.json");
```

The file holds the format, its version and the entities:

```json
{
  "format": "younity-scene",
  "version": 2,
  "entities": [
    {
      "id": 0,
      "components": {
        "Transform": {
          "position": [40.0, 8.0],
          "rotation": 0.0,
          "scale": [1.0, 1.0]
        },
        "Velocity": {
          "value": [100.0, 0.0]
        }
      }
    }
  ]
}
```

A `Vector2f` is `[x, y]`, a `Color` is `[r, g, b, a]` from 0 to 255, a `std::optional<Recti>` is `null` or `[left, top, width, height]`, and a `std::vector<Recti>` is an array of `[left, top, width, height]`. Saving writes the entities by id, each with its id, their components alphabetically and their fields in registration order. Only the components registered in the `ComponentRegistry` are saved: a component type that is not registered is invisible to the file.

Loading is strict. An unknown format, version, component, field or key, a malformed value and a value out of range are errors. A missing field keeps the default value of its component. The whole file is read and checked in a temporary registry before the target is changed, so an invalid file leaves the scene as it was.

A successful load destroys the entities of the registry and creates the ones of the file with their ids, and the registry keeps its systems. An `Entity` the game holds therefore names the same entity after a save and a load, and the systems update the loaded entities at once. `loadScene` must run outside `updateSystems`, because `Registry::destroyEntities` refuses to destroy the entities while a system runs. An id is used once and is at most `MAX_SCENE_ENTITY_ID` (1048575): the registry keeps a slot for each id up to the largest one, so the limit bounds the memory a file makes it take.

The file holds no system, event or other game state, and a resource only by its path. It has no size limit and is read into memory as a whole, and a duplicate JSON key keeps the last value. A syntax error gives its line; a schema error gives line 0, and the message names the JSON path. `SceneFileError` carries the file, the line when there is one, and the reason.

The [editor](editor.md#scene-files) calls these functions from its Scene window and provides a handler to add the systems of a loaded scene.

## Main types

| Type                | Role                                                      |
|---------------------|-----------------------------------------------------------|
| `Entity`            | The id of an entity                                       |
| `Registry`          | Entities, components and systems of one world             |
| `SparseArray`       | The components of one type                                |
| `Signature`         | The set of component types of an entity or of a system    |
| `System`            | Base class of the systems, with a name used by the profiler |
| `Scene`             | A screen of the game, with its registry and its event bus |
| `SceneManager`      | The active scene and the switch from one to another       |
| `ComponentRegistry` | The component types, known by their name                  |
| `ComponentInfo`     | One component type: its fields, read and written by name  |
| `FieldValue`        | The value of a field read or written by name              |
| `FieldType`         | The type of a field, used to check a value read by name   |
| `SceneFileError`    | Why a scene file could not be saved or loaded              |
| `Transform`         | Position, rotation and scale of an entity                 |
| `Velocity`          | Speed of an entity, in pixels per second                  |
| `MovementSystem`    | Moves each entity by its velocity                         |
