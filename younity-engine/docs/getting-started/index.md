# Build the engine

## Requirements

- Linux or Windows. The engine is also developed on macOS.
- A C++23 compiler. The project uses Clang 20, and clang-cl on Windows.
- CMake 3.21 or newer.

The engine uses five libraries: [SFML](https://www.sfml-dev.org) 3 for the window, the graphics, the audio and the input, [Asio](https://think-async.com/Asio/) for the network, [Dear ImGui](https://github.com/ocornut/imgui) with [ImGui-SFML](https://github.com/SFML/imgui-sfml) for the debug interface, [nlohmann/json](https://github.com/nlohmann/json) for the scene files and the plugin configuration, and [Lua](https://www.lua.org) 5.4 with [sol2](https://github.com/ThePhD/sol2) for the scripts. CMake fetches them with [CPM](https://github.com/cpm-cmake/CPM.cmake), so a fresh clone is enough.

On Linux, SFML also needs a few system headers:

```sh
sudo apt install libx11-dev libxrandr-dev libxcursor-dev libxi-dev libudev-dev libgl-dev
```

## Build

=== "Linux and macOS"

    ```sh
    cd younity-engine
    cmake --preset default
    cmake --build --preset default
    ```

=== "Windows"

    ```sh
    cd younity-engine
    cmake --preset default -T ClangCL
    cmake --build --preset default
    ```

The libraries are built in `build/default`, one per module.

## Choose the modules

`logger` and `core` are always built. Each other module has its own switch, `YOUNITY_BUILD_<MODULE>`, turned on by default.

A dedicated server, for example, keeps the game logic and the network, and leaves the graphical modules out:

```sh
cmake --preset default -DYOUNITY_BUILD_RENDER=OFF -DYOUNITY_BUILD_INPUT=OFF -DYOUNITY_BUILD_AUDIO=OFF -DYOUNITY_BUILD_EDITOR=OFF
```

A module depends on the ones listed before it in the [architecture overview](../architecture/index.md): CMake stops with a clear message when a module is built while one it needs is turned off.

## Next steps

- [Run the tests](tests.md)
- [Use the engine in your game](use-in-a-game.md)
