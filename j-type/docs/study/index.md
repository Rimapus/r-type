# Technical study

This section explains the technical choices of the game. It sits on top of the [study of the engine](https://github.com/Rimapus/r-type/blob/main/younity-engine/docs/study/index.md), which covers the language, the libraries, UDP, the packet format and the checks of the network layer: these pages link to it instead of repeating it.

## What the choices answer to

- Up to four players play the same game over the network, on Linux and Windows.
- The server holds the game: it is the only one to decide where the ships are and what the missiles hit.
- The client only shows the game and sends what the player does.
- The game runs at 60 steps a second on both sides, with the fixed step of the engine.

## How to read it

Each topic follows the same order as the engine study: the options, a table, the **choice** and its reason, then its **price**. A number comes from the constants of the code, named next to it.

## The pages

| Page                                            | What it covers                                                        |
|-------------------------------------------------|-----------------------------------------------------------------------|
| [Algorithms and data structures](algorithms.md) | Replication, client display, spawning, collisions                     |
| [Networking](networking.md)                     | Messages, reliability, snapshot rate, bandwidth                       |
| [Storage](storage.md)                           | What the game keeps between two runs, settings and scores             |
| [Security](security.md)                         | Untrusted clients, malformed messages, what the server logs           |

## The choices at a glance

| Topic                    | Choice                                     | Compared with                         |
|--------------------------|--------------------------------------------|---------------------------------------|
| Authority                | The server decides everything              | Client authority, lockstep            |
| Replication              | A full snapshot at each step               | Delta snapshots, events               |
| Client display           | Interpolated, two steps behind; its own ship predicted | As received |
| Inputs                   | The state of the keys at each step         | Key events                            |
| Reliability              | None: every message is a state, sent again | Acknowledgements and resends          |
| Persistence              | The settings of a player in a text file    | JSON, a database                      |
