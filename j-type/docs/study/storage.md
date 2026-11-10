# Storage

The engine study covers what the engine stores: its [resource cache, logs and configuration](https://github.com/Rimapus/r-type/blob/main/younity-engine/docs/study/storage.md). This page covers what the game keeps.

## What the game keeps today

| Data                 | Where                                  | Written by the game |
|----------------------|----------------------------------------|---------------------|
| Sprites              | `assets/`, next to the binaries        | No, read only       |
| Sprite animations    | `assets/data/sprites.txt`              | No, read only       |
| Waves of Bydos       | `assets/data/waves.txt`                | No, read only       |
| Sounds and music     | `assets/audio/`                        | No, read only       |
| Settings of a player | `settings.txt`, in the configuration folder of the user | Yes, by the settings screen |
| Server port          | The command line of the server         | No                  |
| Server address       | The command line of the client         | No                  |
| Game state           | The memory of the server               | No, lost when it stops |

The server writes nothing: it can run from a read-only folder, needs no clean-up, and has no file a player could corrupt. A game lasts until the server stops, and no score outlives it.

## The settings of a player

The settings hold the volumes, the captions, the auto fire, the colorblind correction, the high contrast and the bindings of a player: small, written rarely, and read at start.

| Option              | Fits                             | Reliability                           | Cost                          |
|---------------------|----------------------------------|---------------------------------------|-------------------------------|
| A text file per player | The settings of the client    | Whole if written then renamed         | A parser, no dependency       |
| A JSON file         | Settings and scores              | The same                              | A JSON library in the game    |
| SQLite              | Scores of many games             | Transactions                          | A database library            |

**Choice: one small text file, `settings.txt`, in the configuration folder of the user**: `$XDG_CONFIG_HOME/j-type/` or `~/.config/j-type/` on Linux, `~/Library/Application Support/J-Type/` on macOS, `%APPDATA%\J-Type\` on Windows. Each line is `<name> <value>`, and a missing or invalid value keeps its default. To survive a crash during a write, the file is written under a temporary name, then renamed over the old one: a rename replaces the file in one step, so the old file stays whole until the new one is complete (`saveSettings`).

**Price:** no query and no scores. A file that a player edits by hand is checked when it is read, like any other input, and a value it gets wrong falls back to its default.
