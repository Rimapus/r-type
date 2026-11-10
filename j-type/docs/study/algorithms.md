# Algorithms and data structures

## Authority: the server decides

| Option            | Who computes the game | Cheating           | Latency felt by the player       |
|-------------------|-----------------------|--------------------|----------------------------------|
| Server authority  | The server            | Hard: a client only sends keys | One round trip to see a move |
| Client authority  | Each client, its ship | Easy: a client says where it is | None for its own ship      |
| Lockstep          | Every peer, the same  | Hard, desyncs are visible | The slowest peer sets the pace |

**Choice: server authority.** The subject asks for an authoritative server, and it is the simplest model to keep four players in one game: there is one world, on the server, and the clients draw it. Lockstep needs every peer to compute the same floats in the same order on Linux and Windows, which C++ does not promise across compilers.

**Price:** what a player does reaches the game one round trip late: a few milliseconds on a local network, felt over the Internet. The client hides it for the moves of its own ship by predicting them ([Client display](#client-display-interpolated-its-own-ship-predicted)), but a shot still leaves one round trip after the key.

## Replication: a full snapshot at each step

| Option          | What is sent                                  | A lost datagram          | Code              |
|-----------------|-----------------------------------------------|--------------------------|-------------------|
| Full snapshot   | Every entity, its type and position, each step | Fixed by the next one    | One message       |
| Delta snapshot  | What changed since a snapshot the client acknowledged | Needs the acknowledged base | Acks and history per client |
| Events          | Spawn, move, destroy                          | Lost for good, or resent | Reliability layer |

**Choice: a full snapshot.** At each step the server sends every entity with a `Transform` (`SnapshotMessage`): its id, its type, its position and its health, 14 bytes (`ENTITY_STATE_BYTES`), after the number of the step (`tick`). A snapshot replaces the previous one, so a lost one is made up by the next, 16 ms later. The client keeps the entity of each id while the id is in the snapshots, and drops it when it is gone (`RemoteWorld`).

**Price:** the size grows with the number of entities, even when nothing moves, and one datagram holds 84 entities at most (`MAX_SNAPSHOT_ENTITIES`, from the 1200 bytes of a packet). Past that, the snapshot is split into numbered chunks, up to 64 (`splitSnapshot`), and the client draws it once all its chunks came (`SnapshotAssembler`): a lost chunk loses the whole snapshot of its step. The bandwidth is computed on the [networking page](networking.md#bandwidth).

## Client display: interpolated, its own ship predicted

| Option          | What the client draws                         | Feels like               |
|-----------------|-----------------------------------------------|--------------------------|
| As received     | The positions of the last snapshot            | Exact, with a hitch on a lost snapshot |
| Interpolation   | Between the two snapshots around a step a little behind | Smooth, a few steps later |
| Prediction      | Its own ship moved at once from its keys      | No latency on its ship, corrections when wrong |

**Choice: interpolation, two steps behind, and prediction for its own ship.** UDP can lose a snapshot or deliver two in the wrong order, and drawn as received, the picture then freezes or goes back a step. The client keeps the snapshots in the order of their `tick` (`SnapshotBuffer`) and draws the world two steps before the newest one (`INTERPOLATION_DELAY`): a late snapshot takes its place before it is drawn, a copy is dropped, and a lost one is filled by moving each entity between the snapshot before and the one after. Its own ship is drawn ahead instead (`ShipPrediction`): the client moves it at once with the keys it sends, with the movement code of the server (`moveShip`). Each `Input` has a number, the step of the client (`tick`), and the snapshot of each client gives the number of its last input the server moved the ship with (`lastInput`). From the newest snapshot, the client puts the ship where the server has it and moves it again by the inputs sent since. The server moves the ship with the last keys it got at each step, so a step with no new input, or with two, puts the prediction one step off until the next snapshot.

**Price:** the other entities are drawn two steps, 33 ms, behind the server, plus half a round trip, while the ship of the player is ahead of it: a shot can seem to touch it, or to miss it, before the server decides. When no snapshot comes for more than two steps, the picture stops on the newest one until they come back. When the prediction is one step off, the ship jumps back by one step of movement, 2.5 pixels (`PLAYER_SPEED` over 60 steps), at the next snapshot.

## Spawning the Bydos

| Option             | Where the Bydos come from          | Replay a game     |
|--------------------|------------------------------------|-------------------|
| A timer and a random height | Code                      | With a seed       |
| Waves in a data file | A file, read at start            | With a seed       |
| Scripted levels    | Lua scripts of the engine           | Depends on the scripts |

**Choice: waves in a data file.** The server reads `assets/data/waves.txt` at start: each wave is a pause, then groups of one type of Bydo with a movement pattern, a count, a delay between two Bydos and a formation (`loadWaves`). The `WaveManager` plays them in order, from the moment a player joins. A Bydo appears just past the right edge, so it flies in from off-screen, at a random height or where the formation of its group puts it: a line, a column or a V around a height of the screen. The pause of a wave starts only once the Bydos of the wave before are destroyed or gone, so the waves never pile up; a chasing Bydo flies away after `CHASE_DURATION` for the same reason. A wave can bring the boss, and destroying it wins the game. The last Bydo of a group marked `drop` carries a power-up, drawn from the same seeded generator. A wave also brings its walls and zappers, which scroll to the left with the stage. After the last wave, the waves start again with their delays shorter by `WAVE_SPEEDUP`, down to `MIN_DELAY_FACTOR`, and each player after the first adds a quarter of the Bydos of each group. A game designer changes the waves without building the game again: see [Write a wave](../guides/write-a-wave.md).

**Price:** a wave has no scripted path, only the three patterns and the four formations of the code and the one boss, a wave lasts as long as its slowest Bydo, and the random generator is seeded from the system, so a game cannot be replayed.

## Collisions

**Choice: the `CollisionSystem` of the engine**, which tests every pair of hitboxes at each step and publishes a `CollisionEvent`; the server reads the event and applies the rules of the game. The engine study explains [why every pair](https://github.com/Rimapus/r-type/blob/main/younity-engine/docs/study/algorithms.md#collision-detection-every-pair) is enough for a few hundred hitboxes.

**Price:** the hitbox of each ship and Bydo is a rectangle set by hand to fit its sprite, and a very fast missile could cross a thin Bydo between two steps: at 360 pixels a second (`MISSILE_SPEED`), a missile moves 6 pixels a step, less than the 17 pixels of a Bydo, so it cannot.
