# Logger

The `logger` module writes what the engine and the game are doing, to the terminal and to a file. Every other module links it.

The headers are in `modules/logger/include/younity/logger/`, in the namespace `younity::logger`.

## Writing a message

A logger has a name, the part of the program it speaks for. `get` gives the logger of a name and creates it at the first call.

```cpp
logger::Logger &log = logger::get("game");

log.info("Player {} joined", 2);
log.warn("Packet {} dropped", 12);
```

```text
[11:32:38.827] [INFO    ] [game] Player 2 joined (main.cpp:11)
[11:32:38.827] [WARN    ] [game] Packet 12 dropped (main.cpp:15)
```

A line holds the time, the level, the name of the logger, the message, and the file and line of the call. The message is a format string in the style of `std::format`, checked against its arguments when the code compiles.

## Levels

Each message has a level: `Trace`, `Debug`, `Info`, `Warn`, `Error` or `Critical`. A logger keeps the messages at its own level or above, `Info` by default.

```cpp
log.debug("Wave {} starts", 3);                               // dropped at the Info level

logger::registry().setLevel("game", logger::Level::Debug);
log.debug("Wave {} starts", 3);                               // written
```

A dropped message costs one comparison: its text is built only when it is going to be written. Detailed messages can stay in the code of a system that runs every frame.

`setLevel(level)` with a level alone changes every logger at once.

## Where the messages go

A **sink** is a destination. The messages go to the console from the start, with one color per level when the output is a terminal.

A file is added from the code, or from the environment when the program is started:

```cpp
logger::enableFile("logs/game.log");
```

```sh
YOUNITY_LOG_FILE=game.log ./my_game
```

| Sink          | Destination                                         |
|---------------|-----------------------------------------------------|
| `ConsoleSink` | The terminal                                        |
| `FileSink`    | A file, one line per message                        |
| `MemorySink`  | The memory, to read the messages back in a test     |

A game adds its own destination by writing a class that implements `ISink`, for example an in-game console.

## One logger per module

Each module of the engine has its logger, named after it. CMake generates a small header for every module, `younity/<module>/Log.hpp`, that gives it:

```cpp
#include "younity/ecs/Log.hpp"

younity::ecs::logger().debug("Entity {} created", entity.getId());
```

The name in the line tells where a message comes from, and one module can be made more verbose while the others stay quiet:

```cpp
logger::registry().setLevel("network", logger::Level::Trace);
```

The loggers can be used from several threads at the same time.

## Main types

| Type             | Role                                                       |
|------------------|------------------------------------------------------------|
| `Logger`         | Writes the messages of one named part of the program       |
| `Level`          | Severity of a message                                      |
| `LoggerRegistry` | Creates the loggers, sets their levels, owns the sinks     |
| `ISink`          | A destination for the messages                             |
| `Record`         | One message with its level, time and location              |
