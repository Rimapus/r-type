# Networking

The transport, UDP, and the packet format come from the engine: its study explains [why UDP](https://github.com/Rimapus/r-type/blob/main/younity-engine/docs/study/technologies.md#transport-udp) and how a packet is checked. This page covers what the game puts inside.

## The messages

Each message travels in one `GameData` packet of the engine: an 8-byte header, then one byte for the type of the message, then its fields (`Messages.hpp`).

| Message    | Direction        | When                              | Fields                               |
|------------|------------------|-----------------------------------|--------------------------------------|
| `Join`     | Client to server | Once the session is open, every 0.5 s until the server answers | None |
| `Welcome`  | Server to client | After each `Join`, with a slot free or already held | The slot of the player |
| `Full`     | Server to client | After each `Join`, when the four slots are taken | None                 |
| `Input`    | Client to server | At each step once joined          | The keys held down, one bit each; the step of the client, older than the last one is ignored |
| `Leave`    | Client to server | When the player leaves the game screen | None                                 |
| `Snapshot` | Server to client | At each step                      | The step; the step of the last input of the client; the chunk index and count; the total number of entities; each entity: id, type, x, y, health |
| `Scoreboard` | Server to client | At each step                    | The step; each slot: state (free, playing, out, left), score, lives, weapon; the game over or the victory |
| `Spawn`    | Server to client | When an entity appears            | The step; the entity: id, type, x, y, health |
| `Destroy`  | Server to client | When an entity is gone            | The step; the id of the entity       |
| `PlayerDisconnected` | Server to client | When a player leaves     | The step; the slot of the player     |
| `ScoreEvent` | Server to client | When a player earns points      | The step; the slot; the points earned |
| `DeathEvent` | Server to client | When a ship or a Bydo is destroyed | The step; the id of the entity; the slot of the killer, or none |
| `GameOverEvent` | Server to client | When the game ends           | The step                             |
| `PickUpEvent` | Server to client | When a ship takes a power-up    | The step; the slot; the power-up     |

## Reliability: states, not events

| Option                 | A lost message                        | Cost                                  |
|------------------------|---------------------------------------|---------------------------------------|
| Send states again      | Replaced by the next one              | The same message, every step          |
| Acknowledge and resend | Resent until acknowledged             | Sequence numbers, timers, a queue per peer |
| TCP                    | Resent by the system, the next ones wait | Head-of-line blocking               |

**Choice: every message is a state, sent again; the events only announce a change.** The client sends the keys it holds, not the keys it pressed, so a lost `Input` is replaced 16 ms later by the same information. Each `Input` carries the step of the client, and the server drops one that is not newer than the last it got, so a late one does not bring old keys back. A snapshot is the whole game and the scoreboard is the whole score, so the next one makes up for a lost one. The events carry no state of their own: a `Spawn` or a `Destroy` is a low-latency notice whose outcome the next snapshot already shows, and a `ScoreEvent`, a `DeathEvent`, a `PickUpEvent` or a `GameOverEvent` is repeated by the next snapshot or scoreboard. A lost event never changes the game, but the client plays the sounds, the notices and the points from the events, so a lost one loses its sound or its text. `Join` is the only message that needs an answer, `Welcome` or `Full`, and the client sends it again every 0.5 s (`JOIN_DELAY`) until it gets one. On a `Full`, the client goes back to the main menu, which writes `the server is full`.

**Price:** nothing tells a player that the connection is bad, a key pressed and released between two steps is never seen by the server, and a lost event loses its exact moment. A message that must arrive once, like a game over, is repeated by the scoreboard instead of being acknowledged.

## A player who leaves

A client can crash, freeze or lose the network without saying goodbye, and the game must go on for the others.

| Option                          | A crashed client                  | Cost                                    |
|---------------------------------|-----------------------------------|-----------------------------------------|
| The sessions of the engine      | Dropped after 3 s of silence      | A `Connect` before the `Join`           |
| A timer per client in the game  | Dropped after 3 s of silence      | Code the engine already has             |
| A `Leave` message only          | Keeps its slot forever            | Nothing                                 |

**Choice: the sessions of the engine, and a `Leave` when the player leaves.** The client opens a session (`Session`) before it joins, and the server keeps one per address (`SessionTable`): it ignores the messages of an address without a session, and drops an address silent for 3 s (`DEFAULT_SESSION_TIMEOUT`). The inputs sent at each step keep the session open, so a client goes silent only when something went wrong. The server then removes its ship, and the scoreboard shows the slot as left to the others, with its score, until the next game or a new player takes it. Leaving the game screen, or closing the window, sends a `Leave`, which frees the slot at once. When the last player leaves, a new game starts and stands still until the next player joins. The client watches its own session the same way: when the server stays silent for 3 s, it goes back to the main menu, which writes `the server is unreachable` before it has a slot, and `the connection to the server was lost` after.

**Price:** a crashed player stays 3 s in the game, its ship holding the last keys it sent, and a lost `Leave` costs the same 3 s. A client frozen for more than 3 s loses its slot, and a player waits 3 s before the client tells them the server is gone.

## Snapshot rate

**Choice: one snapshot per step, 60 a second**, the rate of the fixed step (`TICK_STEP`), so the client gets a new state for each step it draws.

**Price:** the bandwidth grows with the rate: half the rate would halve it.

## Bandwidth

A snapshot datagram takes 8 bytes of header, 1 byte of type, a 4-byte step, the 4-byte step of the last input of the client, a 1-byte chunk index, a 1-byte chunk count, a 2-byte total and a 2-byte count, then 14 bytes per entity (`ENTITY_STATE_BYTES`). Up to 84 entities fit in a 1200-byte datagram (`MAX_SNAPSHOT_ENTITIES`); a larger snapshot is split into chunks numbered 0 to `chunkCount - 1`, up to 64 chunks and 5376 entities. UDP and IPv4 add 28 bytes to each datagram.

| Entities in the game | Bytes per snapshot, with UDP and IPv4 | Per client, 60 a second | Server, 4 clients |
|----------------------|---------------------------------------|-------------------------|-------------------|
| 10                   | 163 + 28                              | 11 KB/s                 | 46 KB/s           |
| 30                   | 443 + 28                              | 28 KB/s                 | 113 KB/s          |
| 91 (two chunks)      | 1199 + 28 and 121 + 28                | 83 KB/s                 | 330 KB/s          |

An `Input` is 14 bytes, 42 with UDP and IPv4: about 2.5 KB/s per client to the server. A home connection uploads a few megabits a second, so even a full game of four players fits.

### Measured

The game counts what it sends and receives (`TrafficMeter`), without the 28 bytes of UDP and IPv4 headers of each datagram. Each client shows its traffic each second in the metrics of the debug layout (F1): `network.bytes_sent`, `network.bytes_received` and `network.snapshots`, next to the round trip of the pings (`network.latency`) and the datagrams of the server that never came (`network.packets_lost`). The server logs its traffic every 10 seconds (`TRAFFIC_LOG_PERIOD`):

```text
[INFO    ] [j-type] Traffic with 2 clients: 39328 B/s sent, 1680 B/s received
```

Measured on the loopback of one macOS machine: the server and 1 to 4 clients on `127.0.0.1`, 65 seconds each. Nobody played: for the measure, the ships were made invulnerable (a long `invulnerable` in `spawnPlayer`), so the idle clients stayed in the game while the waves of `assets/data/waves.txt` went on without a Bydo destroyed. The table averages the five full periods of 10 seconds after every client joined.

| Clients | Server sends, average (lowest and highest period) | Per client | Server receives |
|---------|----------------------------------------------------|------------|-----------------|
| 1       | 20.9 KB/s (17.5 to 24.6)                           | 20.9 KB/s  | 0.84 KB/s       |
| 2       | 46.2 KB/s (39.1 to 59.8)                           | 23.1 KB/s  | 1.68 KB/s       |
| 3       | 72.5 KB/s (60.6 to 87.7)                           | 24.2 KB/s  | 2.52 KB/s       |
| 4       | 101.6 KB/s (83.2 to 166.3)                         | 25.4 KB/s  | 3.36 KB/s       |

Each client sends 840 B/s: 60 `Input` of 14 bytes a second, as computed above. Each client receives a little more as the players grow, since each player adds its ship to the snapshots and, after the first, Bydos to the waves; the highest period, with 4 clients, is a wave with many Bydos on screen. Between 20 and 25 KB/s per client sits between the computed snapshots of 10 and 30 entities.

To measure:

```sh
./r-type_server 4242 | grep Traffic        # the server logs its traffic every 10 seconds
./r-type_client 127.0.0.1 4242             # once per client, then PLAY
```

To measure with delay and loss, on Linux, `netem` slows the loopback down for every program, both ways:

```sh
sudo tc qdisc add dev lo root netem delay 50ms loss 1%   # 100 ms of round trip, 1 % of the datagrams lost
sudo tc qdisc change dev lo root netem delay 100ms       # 200 ms of round trip
sudo tc qdisc del dev lo root                            # back to normal
```

The round trip shows in `network.latency`, and the datagrams lost in `network.packets_lost`.
