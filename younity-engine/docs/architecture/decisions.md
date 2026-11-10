# Design decisions

The code shows what the engine does. This page says why it is built this way, for the choices that are hard to guess by reading it. The comparison of each choice with its alternatives is in the [technical study](../study/index.md).

## One library per module

Each module is its own library, and the dependencies go in one direction only. CMake enforces it: the modules are added in order, and a module that asks for one listed after it stops the configuration.

A game links what it needs. A dedicated server builds the game logic and the network and leaves the window, the input and the sound out.

## Third-party libraries stay behind engine types

SFML, Asio and nlohmann/json are used inside the modules and are absent from their public headers. A game opens a `younity::render::Window`, binds a `younity::input::Key`, sends through a `younity::network::UdpSocket` and saves a scene with `younity::ecs::saveScene`.

The game then depends on the engine alone. Replacing a library is a change inside one module, and a game compiles with the engine headers alone.

## Components in sparse arrays

Each component type is stored in an array indexed by the id of the entity. Reading a component is one array access, and the storage fits in a few dozen lines that everyone on the team can follow.

The price is memory: an array is as long as the highest entity id that has the component. Reusing the ids of destroyed entities keeps that length close to the number of entities alive.

## Entity ids are reused

A destroyed entity gives its id back, and the next entity created takes it. A shoot'em up creates and destroys missiles all the time, and this keeps the arrays short.

An id is a plain number, so an id kept after its entity died would point at the entity that took its place. The rule for game code is simple: keep an id only while its entity is alive, and check `isAlive` when in doubt.

## Systems find their entities with signatures

Every entity has a signature, one bit per component type. Every system has one too. The registry updates the list of entities of each system when a component is added or removed, which is rare, instead of searching for them at every frame, which is constant.

A registry handles up to 64 component types, the size of the signature. The number is a constant, `MAX_COMPONENTS`.

## The world can change during a frame

Systems destroy entities and add components while they loop. Two mechanisms make this safe:

- a system receives a copy of its entity list, so its loop is stable;
- an entity destroyed during the update of the systems is removed once all of them have run.

A missile destroyed by the collision system is still there, complete, for the systems that run after it in the same frame. Game code can ignore the order in which entities are destroyed.

## One event bus per scene

A system that subscribes to a bus gives it a function, often one that points back to the system. Each scene owns its bus, and the bus is destroyed after the systems of the scene. A subscription lives exactly as long as the scene, and changing scene clears every subscription at once.

## Game logic in fixed steps

The simulation advances in steps of a fixed duration, whatever the frame rate. The same inputs then give the same result on a fast and on a slow machine, which a networked game needs: the server and the clients have to agree on what happened.

## The engine owns the main loop

The loop lives in `core`, in `Application`, and the game fills its hooks. Every game then gets the same frame: inputs, fixed steps, drawing. A mistake in the order, like updating before reading the inputs or drawing a variable time step, cannot happen.

`core` knows neither the window nor the scenes, so the hooks are where the game connects them. A server and a client share the same loop, and the tests run it with a fake clock.

## Drawing happens once per frame

The systems run at every fixed step, and a frame has zero, one or several of them. Drawing in a system would draw several times in a slow frame and skip a fast one. `drawEntities` is called by the game in `onRender` instead, and finds its entities with `Registry::getEntitiesWith`.

The layer of a sprite is a plain number: the engine does not know the layers of a game, and a number lets the game insert a new one between two others.

## Input through actions

The game asks for actions and the input module maps them to keys, buttons and sticks. Rebinding a control is a change of data, and a game written for the keyboard supports the gamepad by adding bindings.

The devices are read through an interface. The real ones are behind it in the game, and a fake one in the unit tests.

## Resources are shared and loaded by the game

The resource manager gives a resource as a shared pointer to a constant. A sprite that holds its texture keeps it alive, so emptying the cache when the scene changes never leaves an entity with a dangling texture. Ids into a table of resources would be more compact, but they need a check of the generation at every access, which a game with a few dozen files does not need.

The `assets` module reads no file itself: the game gives it one loader per type of resource. The module stays free of the graphics library, a dedicated server uses it without a window, and the unit tests give it fake loaders.

A failed load is not kept in the cache, so a file fixed while the game runs is read at the next try. Paths are put in their normal form and kept inside the asset folder, which makes one file one entry of the cache, whatever way it is written.

## Two ways to report an error

A mistake in the code throws an exception. Asking the registry for a component the entity lacks, or using an entity that was destroyed, is a bug to fix, and the exception names it.

Bad data from outside is returned as a value. A truncated or forged datagram is an ordinary event for a server: the network functions return a `std::expected` or a boolean, and the server drops the packet and carries on.

## Logging that costs little when it is off

A message below the level of its logger is dropped before its text is built. Detailed messages can stay in code that runs every frame, and be switched on for one module when something goes wrong.
