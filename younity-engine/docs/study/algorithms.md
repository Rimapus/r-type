# Algorithms and data structures

The architecture, the data structures and the algorithms of the engine, each compared with its alternatives. The end of the page says which ones are ours and how the numbers were measured.

## Architecture: entity-component-system

|                            | Inheritance tree                              | Objects with components                       | Entity-component-system                            |
|----------------------------|-----------------------------------------------|-----------------------------------------------|----------------------------------------------------|
| A missile is               | A class `Missile`, under `Projectile`, under `GameObject` | A `GameObject` holding a mover and a collider | An id with a `Transform`, a `Velocity`, a `Hitbox` |
| The behaviour lives in     | The virtual methods of each class             | The methods of each component                 | Systems, apart from the data                       |
| A new kind of object       | A new class, to place in the tree             | A new combination of components               | A new combination of components                    |
| The same logic on a server | Each class mixes logic and display            | The display components are left out           | The display systems are left out                   |

**Choice: entity-component-system.**

- The subsystems stay apart: a system receives the entities that have the components it asks for.
- The server runs the same logic: `ecs` and `physics` build with the display modules switched off.
- Components are plain data, ready to send over the network or to save to a file.

**Price:** the logic of one object is spread over several systems, and the order of the systems matters.

## Component storage: sparse arrays

|                       | Sparse array (ours)                                        | Sparse set                                         | Archetypes                                                  |
|-----------------------|------------------------------------------------------------|----------------------------------------------------|-------------------------------------------------------------|
| Layout                | One array per type, indexed by entity id, with empty slots | One packed array per type, and an index to find it | One table per combination of components                     |
| Used by               | Small engines                                              | [EnTT](https://github.com/skypjack/entt)           | [Flecs](https://github.com/SanderMertens/flecs), Unity DOTS |
| Reading one component | One index                                                  | Two indexes                                        | Find the table, then one index                              |
| Memory                | One slot per entity id, up to the highest one used         | One element per component, plus the index          | One row per entity                                          |

**Choice: a sparse array**, a `std::vector<std::optional<Component>>` per type. It gives direct access by entity id in about fifty lines of code. Sparse sets and archetypes are built for worlds far larger than a few hundred entities.

| Measure                                       | Value            |
|-----------------------------------------------|------------------|
| Slot of a `Transform`, a `Velocity`, a `Hitbox` | 24, 12, 20 bytes |
| The three arrays for 1,000 entity ids         | 56 kB            |
| Reading a component from its array            | Under 1 ns       |
| Reading it through `Registry::getComponent`   | About 10 ns      |
| Moving 5,000 entities                         | 0.15 ms          |

**Price:** an array is as long as the highest entity id that received its component, and each access through the registry pays a lookup of the type.

## Matching systems and entities: signatures

|                       | Signatures (ours)                    | Query at each frame                         | Archetypes                           |
|-----------------------|--------------------------------------|---------------------------------------------|--------------------------------------|
| When the work is done | When an entity or a component is added or removed | At every frame, for every system | When an entity or a component is added or removed |
| At each frame         | The list of each system is ready     | Every entity is tested against every system | The tables of each system are ready  |
| Limit                 | 64 component types                   | Open                                        | Open                                 |

**Choice: signatures.** The registry keeps the list of entities of each system and updates it when an entity or a component is added or removed, which is far rarer than a frame.

With one system and half of the entities matching:

| Entities | Signatures | Query at each frame |
|----------|------------|---------------------|
| 200      | 3 µs       | 6 µs                |
| 1,000    | 14 µs      | 30 µs               |
| 10,000   | 150 µs     | 304 µs              |

**Price:** 64 component types per registry, and a copy of the list before each update.

## Communication between modules: an event bus

|                   | Direct calls                          | Event bus, immediate (ours)             | Event queue                             |
|-------------------|---------------------------------------|-----------------------------------------|-----------------------------------------|
| The emitter knows | Each receiver                         | The event                               | The event                               |
| Several receivers | One call each, written in the emitter | Any number, each subscribes on its side | Any number, each subscribes on its side |
| The receivers run | At once                               | At once, inside `publish`               | Later, when the queue is emptied        |
| Between threads   | With locks                            | One thread                              | Its natural use                         |

**Choice: a typed event bus with immediate delivery**, the mediator pattern. `physics` publishes a `CollisionEvent` and the game subscribes to it, each side compiling on its own.

A `publish` takes about 22 ns with one subscriber, against 1 ns for a direct call. A thousand events cost 0.02 ms.

The bus belongs to the thread of the game. The network threads hand their datagrams over through `ThreadSafeQueue`, a queue that the game loop empties.

**Price:** the flow is harder to follow than a call, and a subscription lasts as long as its bus, which is why each scene owns one.

## Collision detection: every pair

|              | Every pair (ours) | Uniform grid               | Quadtree                              | Sweep and prune               |
|--------------|-------------------|----------------------------|---------------------------------------|-------------------------------|
| Pairs tested | All: n(n−1)/2     | The ones sharing a cell    | The ones sharing a region of the tree | The neighbours along one axis |
| To tune      | Zero settings     | The size of a cell         | The depth of the tree                 | The axis                      |
| Code         | Two nested loops  | A structure and its update | A structure and its update            | A sort and a sweep            |

**Choice: every pair**, with the overlap test of two axis-aligned rectangles.

| Hitboxes | Pairs tested | Time per frame | Share of a frame at 60 Hz |
|----------|--------------|----------------|---------------------------|
| 100      | 4,950        | 6 µs           | 0.04 %                    |
| 250      | 31,125       | 33 µs          | 0.2 %                     |
| 500      | 124,750      | 130 µs         | 0.8 %                     |
| 1,000    | 499,500      | 450 µs         | 2.7 %                     |
| 2,000    | 1,999,000    | 1,570 µs       | 9.4 %                     |

By our estimate, a busy screen of a shoot'em up holds a few hundred hitboxes: under 1 % of the frame. A grid would start to pay around 2,000.

**Price:** the time grows with the square of the number of hitboxes, rotation is left aside, and a very fast object can jump over a thin target between two steps.

## Time: fixed steps

|                            | Variable step                            | Fixed step (ours)                            |
|----------------------------|------------------------------------------|----------------------------------------------|
| The simulation advances by | The duration of the last frame           | A constant duration, as many times as needed |
| Same inputs, same result   | Depends on the frame rate                | Yes                                          |
| A slow frame               | One long step, fast objects jump further | Several steps of the usual length            |

**Choice: a fixed step with an accumulator**, the algorithm of [Fix Your Timestep!](https://gafferongames.com/post/fix_your_timestep/). The server and the clients run the same steps and reach the same state.

**Price:** a frame longer than a quarter of a second is cut, and the game slows down for a moment.

## Serialization and packet format

|                          | Raw structs                       | Text, like JSON                           | Schema library, like [Protocol Buffers](https://protobuf.dev) | Field by field (ours) |
|--------------------------|-----------------------------------|-------------------------------------------|-----------------------------------|-----------------------|
| An id and a position     | 12 bytes                          | 29 bytes: `{"id":42,"x":128.5,"y":64.25}` | About 12 bytes                    | 12 bytes              |
| Between two machines     | Padding and byte order can differ | The same text                             | The same bytes                    | The same bytes        |
| Damaged or hostile input | The sizes are trusted             | The parser of the library                 | The generated parser              | Every read is checked |
| Extra tooling            | The standard library              | A JSON library                            | A schema compiler and its library | The standard library  |

**Choice: our own `Writer` and `Reader`**, with one call per field and numbers of fixed size in little-endian. The code and the protocol document say the same thing byte for byte.

The packet header takes 8 bytes:

| Field    | Size    | Why it is there                                              |
|----------|---------|--------------------------------------------------------------|
| Magic    | 2 bytes | Drops the datagrams of other programs at the first two bytes |
| Version  | 1 byte  | Lets a server refuse a client built for another layout       |
| Type     | 1 byte  | Tells game data from the ping and pong of the engine         |
| Sequence | 4 bytes | Lets the receiver see a lost, late or repeated datagram      |

A datagram is 1,200 bytes at most, under the 1,500 bytes an Ethernet frame carries, so it travels in one piece. It holds 99 updates of an id and a position. The [network page](../architecture/network.md#the-packet) gives the full layout.

**Price:** each message is written by hand on both sides, and a value takes whole bytes.

## Classic and custom

Most of the engine uses known algorithms. Four parts are ours.

| Algorithm or structure                  | Origin  | Reference or reason                                                                    |
|-----------------------------------------|---------|----------------------------------------------------------------------------------------|
| Entity-component-system                 | Classic | The usual architecture of data-driven engines                                          |
| Sparse array of optionals               | Classic | The simplest ECS storage                                                               |
| Signatures and one list per system      | Classic | [A Simple Entity Component System](https://austinmorlan.com/posts/entity_component_system/) |
| Event bus                               | Classic | The mediator pattern                                                                   |
| Overlap of two rectangles               | Classic | The axis-aligned bounding box test                                                     |
| Fixed step with an accumulator          | Classic | [Fix Your Timestep!](https://gafferongames.com/post/fix_your_timestep/)                |
| Actions bound to keys and buttons       | Classic | The action mapping of most engines                                                     |
| Queue between the network threads and the game | Classic | A mutex and a condition variable around a `std::deque`                          |
| Destruction at the end of the update    | Custom  | A system can destroy an entity that later systems still hold in the same update        |
| Binary writer, reader and packet header | Custom  | The protocol is ours to design and to document                                         |
| Module order checked by the build       | Custom  | A cycle between two modules fails before anything compiles                             |
| Logger                                  | Custom  | Built on `std::format`, so the base of the engine stays on the standard library        |

## How the numbers were measured

A small benchmark program of our own, linked with the engine and built in Release mode with Clang 21, on a laptop with an Apple M3. Each figure is the mean time of one call over half a second of runs, and an order of magnitude for that machine.

| Measure          | Setup                                                                                            |
|------------------|--------------------------------------------------------------------------------------------------|
| Collisions       | Entities with a `Transform` and a 16×16 `Hitbox`, all apart, `CollisionSystem` alone             |
| Matching         | One entity in two with a `Velocity`, `MovementSystem` against a loop calling `hasComponent`      |
| Component access | 10,000 reads of a `Transform`                                                                    |
| Event bus        | 1,000 calls of `publish`, against direct calls of the same functions                             |
| Build from scratch | Empty dependency cache, default configuration, 8 parallel jobs                                 |
