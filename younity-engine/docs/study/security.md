# Security

The engine is a library, not a service: it holds no account, token or key, and it decides nothing about who may play. Two inputs cross a trust boundary anyway. The first is the datagram that arrives on a socket; the second is the path a loader turns into a file. The engine treats the first as bytes it must not trust and the second as a name it must keep inside one folder. Both checks stop at the edge of the game: what a message means, and which paths the game asks for, are the game's decisions.

## Who is trusted

| Input                            | Comes from                              | What the engine does                                              |
|----------------------------------|-----------------------------------------|-------------------------------------------------------------------|
| Datagram envelope                | Anyone who can reach the UDP port       | Checks size, magic, version, type and payload shape               |
| `GameData` payload               | The same peers                          | Nothing: the bytes are opaque and forwarded                      |
| Peer identity                    | The source address of the datagram      | Compares it with the expected server on the client                |
| Asset path                       | The game's code or data                 | Refuses empty, absolute, folder and climbing paths                |
| File behind the path             | The asset folder                        | Nothing: the loader opens what it receives                        |
| Scene file                       | The game's code or data                 | Checks the format, version, component and field names, and values |
| Log line                         | Every module and the game               | Nothing: the message is formatted as given                        |
| Build inputs                     | Upstream projects and the CI image      | Pins versions and, for some, checks a hash                        |

A server that calls `bind(port)` listens on `0.0.0.0`, so every interface is reachable, and a datagram carries no proof of who sent it: a UDP source address can be written by the sender. The engine therefore never treats an address as an identity, only as a return path.

## The envelope is checked before the game sees it

[`decodePacket`](../architecture/network.md#the-packet) runs on every datagram before a packet reaches the game. It rejects a datagram as soon as one field is wrong:

| Check                                             | Result             |
|---------------------------------------------------|--------------------|
| More than 1200 bytes                              | `TooLarge`         |
| Fewer than the 8 header bytes                     | `Truncated`        |
| Magic other than `0x3659`                         | `BadMagic`         |
| Version other than `1`                            | `BadVersion`       |
| Type other than the six defined values            | `UnknownType`      |
| `Ping` or `Pong` payload not exactly 4 bytes      | `InvalidPayload`   |
| `Connect`, `ConnectAck` or `Heartbeat` with bytes | `InvalidPayload`   |

The payload is then read with [`Reader`](../architecture/network.md#the-serializer), which checks every field against the bytes that remain. An integer that does not fit gives `Truncated`. A boolean other than `0` or `1` gives `InvalidValue`. A string announces a 16-bit length; a length above 256 gives `TooLarge` before anything is allocated, and a length larger than the remaining bytes gives `Truncated`. On the writing side, `Writer` and `encodePacket` return `false` instead of growing past 1200 bytes, so an oversized packet is never cut in half.

**Choice: reject, never repair.** A damaged datagram has no owner and no useful content: dropping it cannot hurt a correct peer, and the server must survive whatever arrives on its port. Returning a value rather than throwing also keeps the receive loop free of exceptions, which is why every check above is an `expected` or a `bool`.

**Price:** these checks cover the envelope only. `GameData` is a byte string to the engine: a well-formed packet can still carry a message that the game cannot decode. The engine also cannot tell a sincere peer from a hostile one, because nothing in the envelope is signed or secret.

### A datagram larger than the receive buffer

The receive thread reads into a buffer of `MAX_PACKET_BYTES`, i.e. 1200 bytes. The operating system then decides what happens to the rest: Linux copies the first 1200 bytes and discards the excess; Windows fails the receive with a message-size error, which the thread logs and drops. In both cases no byte past 1200 reaches the parser. `TooLarge` is still checked in `decodePacket`, because a caller may hand it a larger span directly. An oversized datagram is thus not a parsing problem; on Windows, it also costs a warning line in the log.

## Sessions are liveness, not access control

A client opens a session with `Connect` and keeps it with `Heartbeat`. On the server, `SessionTable` inserts an entry for any `Connect`, keyed by the textual form of the source endpoint, and answers `ConnectAck`; it refreshes the entry on every later packet from that endpoint and `sweep` drops it after 3000 ms of silence. On the client, `Session` ignores every packet whose sender is not the server it connected to, and ignores everything while disconnected or timed out.

That is enough to know who is still there and to time out a peer that vanished. It is not an admission check:

| Property               | Today                                                        | Consequence                                                        |
|------------------------|--------------------------------------------------------------|--------------------------------------------------------------------|
| Who may open a session | Anyone who can send a valid `Connect` to the port            | The port is open to the whole reachable network                    |
| Identity               | The source address and port                                  | Forgeable: UDP carries no credential                                |
| Confidentiality        | None                                                         | Every payload, including a `GameData`, is cleartext on the wire    |
| Integrity              | The UDP checksum only                                         | An on-path attacker can alter a datagram; the envelope still parses |
| Replay                 | `seq` is written but never checked on receipt                | An old `GameData` or `Connect` can be sent again                   |
| Rate limit             | None on the packets; a failing receive is logged once a second | A sender can spend the server's CPU and bandwidth at will          |
| Session table          | At most `MAX_SESSIONS` (1024) peers                          | A flood of spoofed `Connect` fills it and keeps new peers out until `sweep` |
| Receive queue          | At most `DEFAULT_QUEUE_CAPACITY` (1024) datagrams            | A sender faster than the game loop makes the pump drop datagrams   |

**Choice: a handshake for liveness, no access control.** The engine is a library for a small networked game, not a public service. A shared secret or a certificate would have to be distributed, rotated and stored by every game built on the engine, and would still say nothing about what a player may do. The session objects answer the question the game loop actually asks: is this peer still there, and where do I send the next datagram?

**Price:** an attacker who can reach the port can open a session, fill the session table with spoofed endpoints until the sweep frees it, fill the receive queue faster than the game drains it so the datagrams of real players are dropped, or replay a captured packet. None of these requires a key, and none is detected. A game that exposes its server to the open Internet has to put authentication, encryption, rate limiting and bounds in front of the engine, or accept these limits.

## Fuzzing the parser

Inside the engine, the envelope is the only place where bytes from the network are parsed. Besides the unit tests, `fuzz/packet.cpp` feeds arbitrary bytes to `decodePacket` under libFuzzer with AddressSanitizer and UndefinedBehaviorSanitizer. Every accepted packet is re-encoded and decoded again, and the run aborts if the second decode differs in type, sequence or payload. The corpus holds 8 seeds; the run is started by hand:

```bash
cmake --preset fuzz
cmake --build --preset fuzz
mkdir -p build/fuzz/work
./build/fuzz/fuzz_packet build/fuzz/work fuzz/corpus -max_len=1400 -max_total_time=120
```

The 1400-byte maximum is above the 1200-byte limit on purpose: it exercises the `TooLarge` branch as well as the parser.

| Means                  | Input                          | What it looks for                                      | When        |
|------------------------|--------------------------------|--------------------------------------------------------|-------------|
| Network unit tests     | Fixed, hand-written datagrams  | Known cases and regressions                            | Every merge |
| `fuzz_packet`          | Arbitrary bytes                | Crashes, hangs, leaks, sanitizer reports, broken roundtrip | On demand   |
| clang-tidy             | The source                     | Suspicious constructs, not hostile input               | On modules change |

**Choice: fuzz the envelope parser.** It is the one function that reads attacker-controlled bytes on every server. A parser that returns a value for every input and allocates only after a bound check is exactly what a fuzzer can confirm.

**Price:** the harness is not a CI job, so a regression is found only when someone runs it. It sees the envelope and stops there: a malformed `GameData` payload is opaque to the engine and never reaches the parser.

## Asset paths stay inside the asset folder

`normalizeAssetPath` accepts only a relative path that names a file. It refuses an empty path, a path with a root (`/etc/passwd` on Linux; `C:\file` or `\\server\share` on Windows), a path whose normal form is `.`, a path that starts with `..` after normalization, and a path that ends in a separator, i.e. a folder. The loader then receives `assetFolder / normal`, so `sprites/../sprites/ship.png` and `./sprites/ship.png` name the same file and the same cache entry.

| Approach                    | What it stops                                      | What it costs                                                     |
|-----------------------------|----------------------------------------------------|-------------------------------------------------------------------|
| No check                    | Nothing                                            | Any path from a level file or a game message reaches the filesystem |
| Lexical check (ours)        | `..`, absolute paths, folders, empty paths          | Does not resolve links and does not prove the file exists         |
| Canonical path              | The same, plus symbolic links                      | Touches the filesystem, fails on a missing file, races with a swap |
| Container or chroot         | Everything outside the root, links included        | A deployment and a platform-specific setup                        |

**Choice: a lexical check.** It is deterministic, does no I/O, gives the same answer on Linux and Windows, and turns an accidental `../` in a level file into an `InvalidPath` instead of a read outside the assets.

**Price:** the check is on the name, not on the file. A symbolic link, or a Windows junction, placed inside the asset folder is followed by the loader: the lexical path stays inside, the bytes come from outside. Creating that link requires write access to the asset folder, which already lets an attacker replace the assets themselves. The check says nothing about the type or the contents of the file either; that answer belongs to the loader.

## Scene files are checked before they replace the world

`loadScene` reads the file whose path the calling code gives it. The path is not an asset path: it does not go through `normalizeAssetPath`, and the game decides which file to open. The [editor](../architecture/editor.md#scene-files) passes the path typed in its Scene window to the same functions, and the checks below still apply. The engine then checks the content before the registry changes:

| Check                                                              | Result                                  |
|--------------------------------------------------------------------|-----------------------------------------|
| Not valid JSON                                                     | `SceneFileError` with the line          |
| Not an object, or no format, version or entities                    | `SceneFileError`, line 0                |
| Format other than `younity-scene`, or version other than 2          | `SceneFileError`, line 0                |
| Entity id missing, above `MAX_SCENE_ENTITY_ID` or used twice        | `SceneFileError`, line 0                |
| Unknown top-level key, entity key, component or field               | `SceneFileError`, line 0                |
| Wrong value type, array length, integer, color or float range       | `SceneFileError`, line 0                |
| Missing component field                                            | Kept at the default value of the component |
| Valid file                                                         | The entities are replaced, the systems kept |

The whole file is parsed and built in a temporary registry first. Only a complete, valid file changes the target, so an invalid file leaves the old scene untouched. A valid file then replaces the entities of the registry with the ones of the file, under their ids, and the systems stay. Loading must happen outside `updateSystems`. The ids are capped at `MAX_SCENE_ENTITY_ID`, so a file cannot make the registry reserve a slot for an id of four billion.

**Choice: a strict, versioned schema with validation first.** A scene file has no owner on the network and no signature, so the only useful checks are the ones the engine can make locally. Refusing unknown data catches a typo and a file written for another version before it changes the world. Checking in a temporary registry gives the failure a clear answer without a half-built scene.

**Price:** the file is read whole and has no size limit, so a huge file can exhaust the memory of the process. The parser accepts duplicate JSON keys and keeps the last one. The path comes from the caller: the engine does not prove where the file came from or that it was not modified. The schema covers the shape of the data, not its meaning: a valid file can set any registered component to a value the game considers impossible. A resource field, like the texture of a `Sprite`, loads the asset it names through the resource manager, so a scene file can make the game read any file of its asset folder, never one outside it.

## Logs

A log line is formatted as `[time] [level] [category] message (file:line)`, and the message is inserted as given. A newline inside a message therefore starts a new line that looks like a log entry, and a path, a peer name or a game string that reaches a log call can forge one. The file sink is opened in truncate mode by default, writes every line, flushes `Error` and `Critical` at once, and never rotates.

| Risk                    | Today                                                              |
|-------------------------|--------------------------------------------------------------------|
| Forged line             | The message is written verbatim; control characters are not escaped |
| Log flood               | No rate limit; every call writes, including one per failed receive  |
| Disk exhaustion         | No rotation, no size cap                                            |
| Lost tail               | Only `Error` and `Critical` are flushed; `flush` is not `fsync`     |
| Readable by others      | The file gets the default permissions of the process                |
| Secret in a line        | No redaction: whatever the caller passes is written                 |

**Choice: a plain-text line.** It is readable with `grep`, needs no service or format library, and works the same on both platforms. The file is a debugging aid and an operator record, not an audit trail.

**Price:** a server that logs a value controlled by a peer lets that peer add lines to the file. A long-running server can fill its disk, and a power loss can drop the last lines even after a successful write. Anyone who can read the file sees every argument the game logged.

## Dependencies and the build

| Component        | Pin                                                        | Integrity                          |
|------------------|------------------------------------------------------------|------------------------------------|
| CPM              | `0.43.2`                                                   | SHA-256 checked before it runs     |
| SFML             | `3.0.2`, release tarball                                   | SHA-256 checked                     |
| Criterion        | `2.4.3`, Linux release archive                             | SHA-256 checked                     |
| nlohmann/json    | `3.12.0`, release archive                                  | SHA-256 checked                     |
| Asio             | Git tag `asio-1-38-2`                                      | No hash                            |
| Dear ImGui       | `1.91.9b`, release archive                                 | SHA-256 checked                     |
| ImGui-SFML       | `3.0`, release archive                                     | SHA-256 checked                     |
| Lua              | `5.4.9`, release archive                                   | SHA-256 checked                     |
| sol2             | A commit of its main branch, archive                       | SHA-256 checked                     |
| Documentation    | `mkdocs`, `mkdocs-material`, `mkdoxy` at exact versions     | No hash                            |
| CI image         | `ghcr.io/mattkretz/cplusplus-ci/clang20`                   | A tag, resolved when the job starts |
| System packages  | `apt-get install` from the image's repositories            | Whatever the repository serves     |

**Choice: pin and verify what the build downloads.** CPM verifies itself before executing, and every source archive is verified by SHA-256, so a changed file at the same URL fails the configure. The versions are exact, so two clones at different dates get the same sources. In the original repositories a game pinned the engine the same way, by a Git tag in its `CPMAddPackage` call; this repository keeps the engine and the games together.

**Price:** Asio is pinned by a Git tag, which the upstream repository can move; the clone would then accept different code without a warning. The Python packages are exact but unhashed, and the CI image and apt packages are resolved at run time. Nothing scans the dependencies for known vulnerabilities, and a pin is updated by hand.

## What the pipeline checks

The original GitLab CI configuration is not part of this repository; this table records what it ran.

| Job                    | Checks                                                        |
|------------------------|---------------------------------------------------------------|
| `format`               | clang-format on every module source                           |
| `tidy`                 | clang-tidy on the changed module sources                      |
| `build-linux`, `build-windows` | The engine compiles on both systems                    |
| `tests-linux`          | The Criterion cases, with coverage                            |
| `docs`                 | `mkdocs build --strict`, so a broken link fails the job       |

The unit tests run without a sanitizer, and no job runs the fuzzer, a dependency scan or a secret scan. A bug that only appears under AddressSanitizer on a hostile input is not caught by the pipeline.

## What is not covered

| Area                  | State today                                              |
|-----------------------|----------------------------------------------------------|
| Authentication        | None: a `Connect` is accepted from any source             |
| Encryption            | None: payloads and headers are cleartext                  |
| Message integrity     | The UDP checksum only; no signature or MAC                |
| Replay protection     | `seq` is not checked on receipt                           |
| Rate limiting         | None on connections or packets; receive failures are logged once a second |
| Session table         | Bounded by `MAX_SESSIONS`, filled by spoofed `Connect` until the sweep |
| Receive queue         | Bounded by `DEFAULT_QUEUE_CAPACITY`, drops what comes on top |
| Oversized datagrams   | Dropped and counted by `UdpPump`, refused by `decodePacket` |
| Symbolic links        | Followed by the loader                                    |
| Scene file size       | Unbounded; the whole file is read into memory             |
| Scene file origin     | Chosen by the caller; no signature or checksum            |
| Scene file meaning    | Any registered component can be set to any range-valid value |
| Log rotation          | None                                                      |
| Fuzzing               | Manual, not part of a pipeline                            |
| Asio pin              | Git tag without a hash                                    |

The game payload and the game configuration are outside this page. A game defines the messages inside `GameData` and parses its own arguments, like the host and the port of a server; the checks described here end at the packet envelope and the scene file schema, and neither the meaning of a message, the meaning of a component value nor the validity of the game's own settings is an engine guarantee.
