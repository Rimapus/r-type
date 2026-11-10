# Security

The engine checks the envelope of every packet and rejects what it cannot parse, see its [security study](https://github.com/Rimapus/r-type/blob/main/younity-engine/docs/study/security.md). This page covers what the game does with the messages that pass.

## The clients are not trusted

| What a client sends | What the server does with it                                       |
|---------------------|--------------------------------------------------------------------|
| `Connect`           | Opens a session for the address, dropped after 3 s without a datagram |
| `Join`              | Gives the first free slot to a new address, the same slot to a known one; a `Full` when the four slots are taken |
| `Input`             | Keeps the bits of the keys, from a known address only, and only if its step is newer than the last one |
| `Leave`             | Frees the slot of the address and removes its ship                 |
| Anything else       | Ignores it, like any message from an address without a session     |

**Choice: the client only sends its keys.** A client cannot tell the server where its ship is, how much health it has or what it hit: the server computes all of it from the 5 bits of the keys (`Input.hpp`). A modified client can play faster than a person, never more than the rules allow.

An `Input` carries the step of the client. The server keeps the last step accepted for each client, and drops an input that is not newer, so a duplicated or reordered datagram never brings back keys the client already released.

**Price:** a player is known by the address its datagrams come from, and a UDP source address can be written by anyone. Someone who knows the address of a player can send inputs or a `Leave` in their name, or an input with a step far ahead, after which the server drops the inputs of that player, and a server reachable from the Internet can be filled with fake clients until the four slots are taken. A real login, with a secret per player, is out of scope for a game on a local network.

## Malformed messages

**Choice: reject, never repair**, like the engine. `decodeMessage` returns a `DecodeError` for a packet that is not a game message, for an unknown type, for a truncated message, or for a value outside its range. The server skips it, so a malformed datagram has no effect and the receive loop never throws. The rejected datagrams are counted, and their number and the first reason are printed at most once a second (`DropLog`): a flood cannot flood the log too.

A call of the receive loop handles at most 256 datagrams (`MAX_DATAGRAMS_PER_STEP`); the rest waits for the next call, so a flood delays the other clients but never holds a step. The events of a step are capped at 256 as well (`MAX_EVENTS_PER_STEP`): the snapshot and the scoreboard still carry the change the dropped event announced.

**Price:** the count is global: it does not say which address sent what. A per-address rate would need an entry for every address that sends a datagram, and a spoofed source cannot be traced anyway.

## The administration console

The server reads commands from its standard input, its terminal: `players`, `kick`, `restart`, `waves`, `difficulty` and `stop`, with the `help`, `metrics` and `log.level` of the engine. A thread reads the lines (`ConsoleInput`) and the game thread runs them at its next frame, with the console of the engine, and logs the answer.

**Choice: the terminal of the server, never the network.** No message of the protocol reaches the console, so a client, honest or not, cannot run a command: only who started the server, or who can write to its standard input, can. `kick` frees the slot like a `Leave` and ignores the address until its session times out, 3 seconds later; the client then goes back to the main menu, as for a lost server.

**Price:** an administrator needs the terminal of the server, e.g. over SSH, and the console has no history nor completion. A kicked player can join again at once, from a new port: `kick` is not a ban. A line waits for the next frame of the game, and more than 64 lines typed at once (`MAX_CONSOLE_LINES`) lose the ones past them. The thread that reads the terminal is detached: a read blocked on a terminal cannot be interrupted on every system, so it ends with the standard input or with the process.

## What the server logs

The server logs through the logger of the engine, under the name `j-type`: when it starts listening, when a player joins, with its address, when a player leaves, times out or is kicked, when a player is out of lives, the answer of each command of its console, and, as a warning, a summary of the datagrams it rejected at most once a second. The messages go to the terminal, and to a file too when the `YOUNITY_LOG_FILE` variable names one, so a server watched over time keeps its history.

**Price:** there is no rate per address, and the log file is never rotated. Watching a server for a long time needs counters, and the metrics of the engine (`tools`) can hold them.
