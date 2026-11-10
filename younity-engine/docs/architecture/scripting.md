# Scripting

The `scripting` module gives an entity its behaviour from a Lua file: the behaviour changes when the file changes, without compiling the game again.

The headers are in `modules/scripting/include/younity/scripting/`, in the namespace `younity::scripting`. The module is built on `core`, `ecs` and `physics`, for the collisions, and runs the scripts with [Lua](https://www.lua.org) 5.4 through [sol2](https://github.com/ThePhD/sol2). Both stay inside the module: a game includes no Lua header. The saves of the scripts are JSON files, written with [nlohmann/json](https://github.com/nlohmann/json).

A game can be written almost entirely in Lua: its C++ opens the window, connects the input and the audio, and runs the engine, and the scripts do the rest. The [Write a game in Lua](../guides/lua-game.md) guide shows it from start to end. The choice of the library is explained in the [technical study](../study/technologies.md#scripting-lua-54-with-sol2).

## A script

A script is a Lua file that returns a table. The table can have three functions, all optional:

| Function                     | When it runs                                                        |
|------------------------------|---------------------------------------------------------------------|
| `start(entity)`              | Once, at the first step of the entity, before `update`              |
| `onCollision(entity, other)` | Before `update`, once for each collision of the entity, see below   |
| `update(entity, dt)`         | At each step, with the duration of the step in seconds              |

`entity` and `other` are ids of entities, integers.

```lua
-- assets/scripts/spin.lua
local Spin = { time = 0 }

function Spin.start(entity)
    Spin.entity = entity
end

function Spin.update(entity, dt)
    Spin.time = Spin.time + dt
end

return Spin
```

The file runs once for each entity that uses it, so each entity gets its own table: two entities with `spin.lua` each count their own `time`.

## Giving a script to an entity

The `Script` component holds the path of the file, relative to the folder given to the `ScriptSystem`. The system also takes:

- the [component registry](ecs.md) of the game: the scripts reach the components registered in it;
- the event bus the [CollisionSystem](physics.md) publishes on: the scripts get the collisions of their entity;
- what the game gives its scripts, either the `Actions` alone or a `Host`.

A `Host` holds everything the scripts reach outside the registry, as functions the game fills:

| Field     | Type      | What the scripts get                                                          |
|-----------|-----------|-------------------------------------------------------------------------------|
| `actions` | `Actions` | The `input` table: read the actions, bind them and list what is bound         |
| `audio`   | `Audio`   | The `audio` table: play sounds and music                                      |
| `game`    | `Game`    | The `game` table: change scene, quit and set the time scale                   |
| `saves`   | A folder  | The `storage` table: save tables in this folder, nothing is saved when empty  |

`scripting` does not depend on `input` or `audio`: the game connects the functions to its own modules, and an empty function does nothing. A dedicated server gives `Actions{}` and still runs scripts. The component registry, and what the functions capture, must outlive the system.

```cpp
const scripting::Actions actions{
    [&input](const std::string &action) { return input.isActionPressed(action); },
    [&input](const std::string &action) { return input.isActionJustPressed(action); },
    [&input](const std::string &action) { return input.isActionJustReleased(action); },
};

registry.addSystem<scripting::ScriptSystem, scripting::Script>(components, bus, actions, "assets");

const ecs::Entity entity = registry.createEntity();

registry.addComponent(entity, scripting::Script{"scripts/spin.lua"});
```

`scripting::registerComponents` registers `Script` with its `path`, so a [scene file](ecs.md#scene-files) and the [editor](editor.md) read and write it like any other component.

## Reaching the entities and their components

A script reaches the entities through the global `ecs` table. A component is named as in the component registry, and a field as in its `field()` call:

| Function                       | What it does                                                         |
|--------------------------------|----------------------------------------------------------------------|
| `ecs.create()`                 | Creates an entity without any component and returns its id           |
| `ecs.destroy(entity)`          | Destroys the entity once all the systems of the step have run        |
| `ecs.isAlive(entity)`          | Tells if the entity exists, false for a value that is not an id like `nil` or `-1`; a destroyed one stays alive until the end of the step |
| `ecs.has(entity, name)`        | Tells if the entity has the component                                |
| `ecs.add(entity, name)`        | Adds the component with its default values, replacing the one it has |
| `ecs.remove(entity, name)`     | Removes the component, does nothing if the entity has none           |
| `ecs.get(entity, name)`        | Returns a copy of the fields of the component, in a table            |
| `ecs.set(entity, name, fields)`| Writes the fields given in the table, the others keep their value    |

```lua
-- assets/scripts/move_right.lua
local Move = {}

function Move.update(entity, dt)
    local transform = ecs.get(entity, "Transform")

    ecs.set(entity, "Transform", { position = { x = transform.position.x + 60 * dt, y = transform.position.y } })
end

return Move
```

`ecs.get` returns a copy: changing the table changes nothing until it is given to `ecs.set`. The fields are converted like this:

| Field type            | In Lua                                           |
|-----------------------|--------------------------------------------------|
| `bool`                | A boolean                                        |
| An integer            | An integer, a number like `3.0` is accepted      |
| `float`               | A finite number                                  |
| `std::string`         | A string                                         |
| `Vector2f`            | `{ x = ..., y = ... }`                           |
| `Color`               | `{ r = ..., g = ..., b = ..., a = ... }`, a missing channel is 255 |
| `std::optional<Recti>`| `nil`, or `{ left = ..., top = ..., width = ..., height = ... }` |
| `std::vector<Recti>`  | An array of `{ left = ..., top = ..., width = ..., height = ... }` |

A field set to `nil` is absent from a Lua table, so `ecs.set` cannot give it no value: an optional rectangle keeps the one it has.

`ecs.set` checks every field before it writes the first one: a wrong field leaves the component unchanged. An unknown component or field, an entity that is not alive, a component the entity does not have or a value of the wrong type raises a Lua error, with the file and the line of the call. Like any error, it stops the script unless the script catches it with `pcall`.

An id is reused once its entity is destroyed: a script that keeps the id of another entity checks it with `ecs.isAlive` before each use, and can still reach a new entity that took the id.

## Collisions

The `ScriptSystem` listens to the `physics::CollisionEvent`s of the event bus. At its next step, it calls `onCollision(entity, other)` for each collision of an entity, whichever of the pair the entity is, before its `update`:

```lua
local Missile = {}

function Missile.onCollision(entity, other)
    ecs.destroy(entity)
end

return Missile
```

A collision published during a step, after the `ScriptSystem` ran, reaches the scripts at the next step. Each published collision is given once, and the CollisionSystem publishes a pair at each step while it overlaps: two entities that stay in contact call `onCollision` at each step.

A collision goes to the script the entity had when it was published. One published before the script started, or before the id of a destroyed entity was given to a new one, is not given.

Once a script destroys its entity with `ecs.destroy`, or removes its `Script`, it gets no other collision and no `update` in the step. An error raised by `onCollision` stops the script like any other error, and `update` is not called.

## Actions and logs

The `input` table reads the actions given to the `ScriptSystem`, usually the ones of the [InputManager](input.md), by their name, and binds them:

| Function                              | What it does                                                        |
|---------------------------------------|---------------------------------------------------------------------|
| `input.isPressed(action)`             | Tells if the action is held down                                    |
| `input.isJustPressed(action)`         | Tells if the action has just been pressed                           |
| `input.isJustReleased(action)`        | Tells if the action has just been released                          |
| `input.bindKey(action, key)`          | Binds a key by its name, e.g. `"Space"` or `"LShift"`               |
| `input.bindButton(action, button)`    | Binds a gamepad button by its number, from 0                        |
| `input.bindAxis(action, axis, side)`  | Binds a side of a gamepad axis, e.g. `"X"` and `-1` for the left of the stick |
| `input.unbind(action)`                | Removes everything bound to the action                              |
| `input.getBindings(action)`           | Returns what is bound, as names for a player, e.g. `{ "Space", "Button 0" }` |

The names of the keys and axes are the ones of the [input module](input.md#actions-and-bindings). An unknown key or axis, a button that is not an integer from 0, or a side that is not `1` or `-1` raises an error. Without input, nothing is bound and `input.getBindings` returns an empty table.

The `log` table writes a string to the logger of the module, named `scripting`, at a level: `log.debug`, `log.info`, `log.warn` and `log.error`. The message starts with the file and the line of the call:

```lua
if input.isJustPressed("shoot") then
    log.info("shoot")
end
```

```text
[INFO    ] [scripting] .../assets/scripts/ship.lua:12: shoot
```

## Sounds and music

The `audio` table plays the sounds and the music of the game, by their path in the assets:

| Function                            | What it does                                                     |
|-------------------------------------|------------------------------------------------------------------|
| `audio.play(path, volume)`          | Plays a sound once, at a volume from 0 to 1, 1 when not given    |
| `audio.playMusic(path, looping)`    | Plays a music in place of the one playing, looping when not given |
| `audio.pauseMusic()`                | Pauses the music                                                 |
| `audio.stopMusic()`                 | Stops the music                                                  |
| `audio.setVolume(volume)`           | Sets the volume of all the sounds, from 0 to 1                   |
| `audio.setMusicVolume(volume)`      | Sets the volume of the music, from 0 to 1                        |

```lua
if input.isJustPressed("jump") then
    audio.play("sounds/jump.wav", 0.8)
end
```

A path must stay inside the assets: an absolute path or one with `..` raises an error, and so does a volume outside 0 to 1. The game loads and plays the files, e.g. with the `SoundPlayer` and `Music` of the [audio module](audio.md).

## Scenes, quitting and time

The `game` table asks the game loop:

| Function                    | What it asks                                                              |
|-----------------------------|---------------------------------------------------------------------------|
| `game.loadScene(path)`      | Replace the entities with the ones of a [scene file](ecs.md#scene-files) of the assets |
| `game.quit()`               | Stop the game                                                             |
| `game.setTimeScale(scale)`  | Multiply the duration of the next steps: 0 freezes the game, 1 is the normal speed |

`ecs::loadScene` cannot run while the systems update, so the game keeps the path and loads the scene once the step is over: the scripts of the old scene finish their step, and the ones of the new scene start at the next one. A scene path follows the rule of the audio paths, and a negative or infinite scale raises an error.

## Modules

`require(name)` runs a Lua file of the folder of the scripts and returns what it returns, like the `require` of Lua: `require("scripts.rooms")` runs `scripts/rooms.lua`. A module shares code and data between scripts:

```lua
-- assets/scripts/game.lua
local rooms = require("scripts.rooms")
```

A module runs once: every script that requires it gets the same value, and a module that returns nothing gives `true`. A name is made of words of letters, digits and `_`, separated by dots, so a module stays inside the folder of the scripts. A missing module, a module that fails, or one that requires itself raises an error in the script that required it.

## Saves

The `storage` table saves a table to a JSON file and loads it back, e.g. the best times or the settings:

| Function                     | What it does                                                                |
|------------------------------|-----------------------------------------------------------------------------|
| `storage.save(name, table)`  | Writes the table to `<name>.json` in the folder of the saves, and tells if it did |
| `storage.load(name)`         | Returns the table of the save, or `nil` when there is none or it is not valid JSON |

```lua
local best = storage.load("best") or { time = math.huge }

if time < best.time then
    storage.save("best", { time = time })
end
```

A name has up to 64 letters, digits, `_` and `-`. A save holds booleans, numbers, strings and tables, with either the keys 1 to n or string keys; anything else, or a table that contains itself, raises an error. The [storage study](../study/storage.md#script-saves-json-in-a-folder-of-the-game) explains the format. Without a folder, `storage.save` returns `false` and `storage.load` returns `nil`.

## What the system does at each step

For each entity with a `Script`, the `ScriptSystem`:

1. starts the script if the entity is new, if its `path` changed or if its file was saved again: it runs the file and calls `start`;
2. calls `onCollision(entity, other)` for each collision of the entity;
3. calls `update(entity, dt)`.

Then it frees the state of the entities that lost their `Script` or were destroyed.

An id is reused after its entity is destroyed, so the system cannot tell a new entity from the old one by its id. `Script::instance` tells them apart: the system writes a number in it when the script starts, and a new `Script` starts at `0`.

## When a script fails

A missing file, a syntax error, a file that does not return a table, a `start`, `update` or `onCollision` that is not a function, or an error raised while it runs: the error is logged once, with the file and the line, and the script stops for this entity. The other scripts and the game go on. `isRunning(entity)` tells if the script of an entity runs.

```text
[ERROR   ] [scripting] script 'scripts/spin.lua' of entity 3 stopped: .../assets/scripts/spin.lua:7: attempt to perform arithmetic on a nil value
```

The script does not start again until its file is saved again, its `path` changes or its entity is destroyed.

## Editing a script while the game runs

At each step, the `ScriptSystem` reads when the file of each script was last written, once per file. When a file is saved again, every script using it starts again at the next step: the file runs again, each entity gets a new table and `start` is called. The same goes for the modules a script required as it started: saving `scripts/rooms.lua` starts again every script that required it, and their `require` runs the new file. What the old table held is lost, and a script stopped by an error runs again once its file is fixed and saved.

A file saved with an error stops the script like at its first start, until the next save. A missing file that is created later starts its scripts too. A file that goes missing is not a change: some editors delete the file for a moment while they save it, so its scripts keep running until it comes back.

## What a script can use

All the scripts run in one Lua interpreter, owned by the `ScriptSystem`, with the `base`, `coroutine`, `math`, `string`, `table` and `utf8` libraries. `io`, `os` and `package` are left out, and so are `dofile` and `loadfile` of the base library: a script cannot open files or run programs. It loads the modules of the folder of the scripts with `require`, and writes files only through the `storage` table, in the folder of the saves. The engine reads `start` and `update` from the table directly, without its metatable.

The scripts share the global variables of the interpreter: a variable assigned without `local` is seen by every script. Nothing limits the time a script takes either: a script that never returns, like `while true do end`, freezes the game.

The `ScriptSystem` is a system like the others, see [A frame, step by step](frame.md#inside-a-step): the scripts run during the fixed step, at the place the game added the system. Like the rest of the engine, it is not thread-safe: the scripts run on the thread of the game loop.
