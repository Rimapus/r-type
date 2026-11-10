# Getting started

This page builds J-Type and starts a game.

## Requirements

- A C++23 compiler. We use Clang 20, and clang-cl on Windows.
- CMake 3.21 or newer.

CMake fetches the libraries of the engine with CPM. The engine and J-Climb are in the same repository, next to this folder, so a fresh clone is enough. On Linux, SFML also needs a few system headers:

```sh
sudo apt install libx11-dev libxrandr-dev libxcursor-dev libxi-dev libudev-dev libgl-dev
```

## Build

```sh
cmake --preset default
cmake --build --preset default
```

The `default` preset builds the engine from `../younity-engine` and J-Climb from `../j-climb`.

The two binaries, `r-type_server` and `r-type_client`, are written at the root of `j-type/`, next to the `assets` folder they read.

## Play

Start the server on a UDP port, then one client per player with the address of the server and the same port:

```sh
./r-type_server 4242
./r-type_client localhost 4242
```

A difficulty can follow the port: `./r-type_server 4242 easy` gives each player 5 lives and Bydo shots 30% slower and two thirds as often; `hard` gives 2 lives and shots 25% faster, a third more often; `normal`, the default, gives 3 lives. The `difficulty` command of the console of the server changes it while it runs.

The client opens on the main menu: PLAY joins the server, HELP recalls the goal of the game and its controls with the keys bound, SETTINGS sets the volume of the music and of the sounds. A game holds four players: each client gets the first free ship. The host is a name, like `localhost`, or an address. `Ctrl+C` stops the server cleanly: it ends the loop, then closes its socket and its network threads.

| Key        | Gamepad       | Action |
|------------|---------------|--------|
| Arrow keys | Left stick    | Move   |
| Space      | First button  | Shoot  |
| M          | Fourth button | Mute or unmute the sound |
| Escape     | Second button | Leave the game |
| F1         |               | Show or hide the debug windows of the engine |
| F2         |               | Show or hide the editor of the engine |

On the main menu, the Konami code, up up down down left right left right with the arrow keys or the left stick, starts [J-Climb](https://github.com/Rimapus/r-type/blob/main/j-climb/README.md), a platformer built on the same engine and packaged with the game. It opens in its own window, and J-Type keeps running behind it.

These are the defaults: SETTINGS, then CONTROLS, binds each action to another key and another gamepad input; RESET gives the defaults back. Escape and the second button stay kept for going back, F1 and F2 for the windows of the engine. In the menus, the arrow keys or the left stick move, Enter or the first button confirms, Escape or the second button goes back, whatever the bindings. SETTINGS also switches on the captions of the sounds, for the players who cannot hear them, the auto fire, which shoots without holding the key, the correction of the colors of the window for protanopia, deuteranopia or tritanopia, the high contrast, which darkens the starfield, and REDUCE FLASHES, which draws the explosions smaller and dimmer. The settings are kept in `settings.txt`, in `~/.config/j-type/` on Linux, `~/Library/Application Support/J-Type/` on macOS and `%APPDATA%\J-Type\` on Windows.

## Run the tests

```sh
cmake --preset tests
cmake --build --preset tests
```

The `tests` preset builds the Criterion tests and runs them.

## Fuzz the messages

The message codec has a LibFuzzer harness, built with Clang:

```sh
cmake --preset fuzz
cmake --build --preset fuzz
mkdir -p build/fuzz/work
./build/fuzz/fuzz_messages build/fuzz/work fuzz/corpus -runs=1000
```

`fuzz/corpus` holds the seed datagrams. The fuzzer writes the inputs it finds to the folder given first, so that one stays in `build/`.
