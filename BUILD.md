# Build and run

The three folders are separate CMake projects. J-Type and J-Climb add the engine from `../younity-engine` with CPM, so keep them next to each other.

## Requirements

- Linux, macOS or Windows.
- CMake 3.21 or newer.
- A C++23 compiler. The project uses Clang 20, and clang-cl on Windows.
- Network access on the first configure: CPM fetches SFML 3.0.2, Asio, Dear ImGui with ImGui-SFML, nlohmann/json, Lua 5.4 and sol2.

On Linux, SFML also needs a few system headers:

```sh
sudo apt install libx11-dev libxrandr-dev libxcursor-dev libxi-dev libudev-dev libgl-dev
```

On Windows, configure with `-T ClangCL`.

## J-Type and J-Climb

```sh
cd j-type
cmake --preset default
cmake --build --preset default
```

The build writes `r-type_server`, `r-type_client` and `j-climb/j-climb` in `j-type/`, next to the assets they read.

Start a server, then one client per player:

```sh
./r-type_server 4242
./r-type_client localhost 4242
```

The `release` preset builds the same binaries optimized:

```sh
cmake --preset release
cmake --build --preset release
```

## J-Climb standalone

From the repository root:

```sh
cd j-climb
cmake --preset default
cmake --build --preset default
./j-climb
```

The binary is written next to its `assets/` folder.

## Tests

Each `tests` preset builds the Criterion suite and runs it. On Linux, CPM downloads Criterion with the other dependencies; on macOS, install it first with `brew install criterion`. Run each block from the repository root.

```sh
cd younity-engine
cmake --preset tests
cmake --build --preset tests
```

```sh
cd j-type
cmake --preset tests
cmake --build --preset tests
```

```sh
cd j-climb
cmake --preset tests
cmake --build --preset tests
```

## Documentation

The J-Type and engine documentation sites are built with MkDocs from their `docs/` folder. Run each block from the repository root:

```sh
cd j-type
pip install -r docs/requirements.txt
mkdocs serve
```

```sh
cd younity-engine
pip install -r docs/requirements.txt
mkdocs serve
```

The engine site also generates its API reference with Doxygen. Use `mkdocs build --strict` to check every page and link.

## Troubleshooting

- If the configure step cannot find the engine, run CMake from `j-type` or `j-climb` and check that `../younity-engine` exists.
- If SFML fails on Linux, install the system headers above and configure again in a fresh build folder.
- If a test binary is missing, check that the `tests` preset was configured before its build preset.
