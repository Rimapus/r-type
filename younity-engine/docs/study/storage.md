# Storage

The engine handles five kinds of data: the resources it loads, the logs it writes, the configuration that tells it how to run, the scenes it saves, and the tables its scripts save. Each has a different lifetime, so each uses a different storage. This page compares the techniques, then says what the choice costs.

## Resources: files on disk, cache in memory

An asset is a file of the game, e.g. a texture, a font or a sound. The [`assets`](../architecture/assets.md) module does not open files itself: the game gives a `Loader<T>` per type, and `ResourceCache<T>` calls it the first time a path is asked for. `ResourceManager` owns one cache per type, all under the same asset folder. The engine provides the loaders of `render::Texture` and `render::Font`; a game can give its own.

|                            | Preload everything at startup        | On-demand cache (ours)                 | Stream on each use                  |
|----------------------------|--------------------------------------|----------------------------------------|-------------------------------------|
| When the file is read      | Once, before the game starts         | The first time a path is asked for     | At every use                        |
| Memory                     | Every file, used or not              | Every file loaded so far               | One file at a time                  |
| First use                  | No wait                              | The wait of the read                   | The wait of the read                |
| Unused assets              | Still in memory                      | Never read                             | Never read                          |
| Failure                    | The game cannot start                | Returned for that path                 | Returned at each use                |
| Code                       | A loading screen and a list of files | A cache per type                       | A loader called by the drawing code |

**Choice: an on-demand cache.** The game has a few dozen files, and the same texture is used by many entities: a missile, a player and an explosion can share one image. Reading it once and keeping it avoids both a loading screen and a read per sprite. A failed load is not kept, so a file fixed while the game runs is read again at the next try.

**Price:** the cache has no eviction and no size limit. A game that loads an unbounded set of paths grows until it calls `unload` or `clear`. `ResourceManager` and `ResourceCache` are not safe to use from several threads at once, and a loader that throws makes the exception go through `load`. The first use of a large file is a wait in the frame that asks for it.

### What the cache keeps

`load` returns a `std::shared_ptr<const T>`. The cache and every part of the game that uses the resource share it, and the resource is destroyed when the last of them lets it go.

|                            | A copy returned by value              | `shared_ptr<const T>` (ours)             | An id in a table                     |
|----------------------------|---------------------------------------|------------------------------------------|--------------------------------------|
| Who owns the resource      | The caller                            | The cache and every user                 | The table                            |
| After `unload` or `clear`  | The copy is still valid               | Valid while a user holds it              | The id can point at another resource |
| Memory                     | One copy per user                     | One object, with a reference count       | One object, plus the table           |
| Changing a resource        | Each user has its own copy            | Impossible through the pointer           | Possible through the table           |

**Choice: a shared pointer to a constant.** Changing scene can empty the cache without checking what the entities of the old scene still hold: a sprite that keeps its texture keeps it alive. Read-only access also means one entity cannot change the texture of all the others.

**Price:** each resource carries a reference count and each user a pointer. The cache cannot reclaim a resource that a scene still holds, even after `unload`.

### Paths and cache keys

A path is relative to the asset folder. `normalizeAssetPath` puts it in normal form, so `sprites/ship.png`, `./sprites/ship.png` and `sprites/enemies/../ship.png` are one file and one entry of the cache. An empty path, an absolute path, a folder, or a path that climbs out with `..` is refused with `AssetError::InvalidPath`, and the loader is not called.

The check is lexical: it does not resolve symbolic links. A link placed inside the asset folder is followed by the loader. The [security page](security.md#asset-paths-stay-inside-the-asset-folder) says what this does and does not cover.

### What the cache does not do

There is no eviction policy, no maximum size, no cache on disk, and no watch on the files: changing a texture on disk does not reload it. `setLoader` replaces the loader of a type and drops the resources loaded with the old one; `clear` empties every cache and keeps the loaders. A game that needs a bounded cache or a hot reload builds it on top of `ResourceCache`, or replaces it.

## Scene files: JSON with a version

A scene file holds the entities and components of an ECS registry. `saveScene` writes it from a `Registry` and a `ComponentRegistry`, and `loadScene` reads it back.

|                            | Custom text                          | JSON (ours)                              | Binary                               |
|----------------------------|--------------------------------------|------------------------------------------|--------------------------------------|
| Read and edited by hand    | Yes, with a parser to write          | Yes, with the format and a schema        | No                                   |
| Value types                | Ours to parse                        | Boolean, integer, float, string and array | Ours                                |
| Library                    | None                                 | nlohmann/json, header-only               | A serializer to write                |
| File size                  | Larger than binary                   | Larger than binary                       | The smallest                         |
| Reviewed in a merge request| Yes                                  | Yes                                      | No                                   |
| Unknown data               | Ours to refuse                       | Ours to refuse, field by field           | Ours to refuse                      |

**Choice: JSON through nlohmann/json.** A scene file is data that a person edits and reviews, like a level in a merge request. JSON brings the values and the parser; `loadScene` keeps the schema: it checks the format, the version, every component and field name, and every value before it changes the registry. The library is header-only, so it adds no build step, and its release archive is pinned by SHA-256.

**Price:** a scene file is larger than a binary one. The whole file is read into memory and has no size limit, and an unexpected file can spend the memory of the process. The schema is code, not a schema language: version 1 is the only accepted version, and an unknown version is refused rather than migrated. Only the components registered in the `ComponentRegistry` are saved and loaded; an unregistered component is invisible. Entity ids, systems, events and resources are not stored, so the ids start again at 0 and the caller adds the systems after a load. Duplicate JSON keys keep the last value. A failed save can leave a partial file, since the write is not atomic.

## Script saves: JSON in a folder of the game

A script saves a Lua table, e.g. its best times or its settings, with `storage.save(name, table)` and reads it back with `storage.load(name)`, see [Scripting](../architecture/scripting.md#saves). The game gives the folder; the engine picks no place of its own.

|                            | One JSON file per save (ours)        | One file for every save              | SQLite                               |
|----------------------------|--------------------------------------|--------------------------------------|--------------------------------------|
| Read and edited by hand    | Yes                                  | Yes, a larger file                   | With a tool                          |
| Value types                | Boolean, number, string and table    | The same                             | Columns, a table to map by hand      |
| Library                    | nlohmann/json, already in the engine | nlohmann/json                        | A new dependency                     |
| A failed write             | Loses nothing: written beside, then renamed | Loses every save if not atomic | Rolled back by a transaction         |
| Two saves at once          | Separate files                       | One file to share                    | Handled by the database              |

**Choice: one JSON file per save, written atomically.** The values of a Lua table map to JSON one to one, the library is already used by the scene files, and the file stays readable when a player or a tester opens it. Each save is written to `<name>.json.tmp`, then renamed over `<name>.json`, so a crash while writing keeps the previous save. The name is checked, letters, digits, `_` and `-` only, so a script cannot write outside the folder.

**Price:** a table has either the keys 1 to n, saved as an array, or string keys, saved as an object; a table mixing both, a function or a table containing itself raises an error. An empty table comes back as an empty table either way. Numbers keep their kind, integer or float, but the whole file is read into memory and has no size limit. There is no version and no migration: a game that changes the shape of a save checks it after `storage.load`. The rename replaces the file on Linux and Windows, but nothing calls `fsync`.

## Logs: console now, file on demand

The [`logger`](../architecture/logger.md) module writes one line per message to a sink. The console sink is always there; a file sink is added from the code with `enableFile`, or from the environment with `YOUNITY_LOG_FILE`. A line holds the local time, the level, the logger name, the message and the source location.

|                            | Console only                          | Console and file (ours)                   | System log, e.g. syslog or journald | Rotating file                         |
|----------------------------|---------------------------------------|-------------------------------------------|-------------------------------------|---------------------------------------|
| Where the lines go         | The terminal                          | The terminal and a file                   | The system logging service          | A file, with several generations      |
| Kept after a crash         | Scrollback only                       | Buffered lines can be lost; errors are flushed | Depends on the service         | Depends on the flush and the rotation |
| Disk growth                | None                                  | Unbounded unless the operator manages it  | Managed by the service              | Bounded by the rotation policy        |
| Setup                      | None                                  | A path or an environment variable         | A platform API or its configuration | A rotation policy and more code       |
| Read from another machine  | Capture the terminal                  | Copy or collect the file                  | Native tools                        | Copy or collect the files             |

**Choice: the console and an optional file.** The console is enough to develop; the file is what a dedicated server or a test job keeps. It needs no service and works the same on Linux and Windows. `Error` and `Critical` lines are flushed at once, so the reason of a crash is on disk even when the program stops right after.

**Price:** there is no rotation and no size limit: a long-running server can fill its disk. A flush is not `fsync`, so the last lines can still be in the operating system cache when the machine loses power. The file is text, not a structured format, and its permissions are the default ones of the process, not a restricted mode. A message argument that contains a newline adds a line that looks like a log entry. The time is local, without a time zone, and a line has no thread or process id.

`MemorySink` keeps the same records in memory for the tests. It is not a production destination.

## Configuration: build options, environment, arguments

The engine has no configuration file. What it reads at run time is small:

|                            | Command-line arguments               | Environment variables (ours)             | Configuration file                  | Settings service                     |
|----------------------------|--------------------------------------|------------------------------------------|-------------------------------------|--------------------------------------|
| Where it lives             | The process invocation               | The process environment                  | A file                              | An external service                  |
| Persistent                 | No, apart from the shell history     | No, apart from the shell profile         | Yes                                 | Yes                                  |
| Typed and validated        | By the parser of the game            | As text, at the point of use             | By a parser or a schema             | By the service schema                |
| Reload without restart     | No                                   | No                                       | Possible                            | Possible                             |
| Discoverable from the code | Through the usage message            | Through the documentation and the source | Through the file itself             | Through the service                  |
| Failure                    | The game exits with a usage message  | A default, or an error in the log        | A parse error                       | A network or authentication error    |

**Choice: command-line arguments for the game, environment variables for the engine.** The engine reads `YOUNITY_LOG_FILE` to add a file sink and `NO_COLOR` to disable the colors of the console. J-Type reads its host and port from its client arguments, and its port from the server arguments. This keeps the surface tiny, adds no parser or configuration dependency, and fits containers and CI, where the environment is the natural place for a path.

**Price:** there is no schema, no single place that lists every setting, and no reload. An environment variable is an API without a type: a typo silently leaves the default, and an empty `YOUNITY_LOG_FILE` is ignored. The engine has no user settings of its own: a game keeps them in a [script save](#script-saves-json-in-a-folder-of-the-game).

The build options are separate: `YOUNITY_BUILD_<MODULE>` and the presets decide which modules are compiled and how. They are read by CMake at configure time, not by the engine at run time.

## What is not stored

The engine writes three kinds of file: the log file, the scene file and the saves of the scripts. It has no replay, no scoreboard, no user profile and no resource manifest. The `Writer` and `Reader` of the network are a wire format, not a file format: they have no header of their own, no version of the file and no checksum.

Game state lives in the ECS in memory. A scene file stores the entities and their registered components, not the systems, the events, the resources or the other state of the game. A game that wants to save more than a scene, like a score, saves it from its scripts with the `storage` table, or defines its own format; the engine gives it the components, the binary serializer and the script saves, not a persistence policy.
