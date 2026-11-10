# Technologies

The language, the libraries and the tools of the engine, each compared with its alternatives.

## Language standard: C++23

| Standard | What the engine takes from it                                               | Where                                     |
|----------|-----------------------------------------------------------------------------|-------------------------------------------|
| C++17    | `std::optional`, `std::string_view`, `std::filesystem`                      | Component storage, serializer, logger, resources |
| C++20    | Concepts, `std::span`, `std::format`, `std::source_location`, `operator<=>` | Math types, serializer, logger, entities  |
| C++23    | `std::expected`                                                             | Socket, reader, packet decoder, resource loading |

**Choice: C++23**, for `std::expected`. A function that reads from the network or loads a resource returns its result or the reason of the failure, as a value.

**Price:** a recent compiler, Clang 20 on Linux and clang-cl on Windows.

## Build system: CMake

|             | [CMake](https://cmake.org)        | [Xmake](https://xmake.io)         | [Meson](https://mesonbuild.com) |
|-------------|-----------------------------------|-----------------------------------|---------------------------------|
| Build files | CMake language                    | Lua                               | Meson language                  |
| SFML        | Built by its official build files | A package of the Xmake repository | A wrap of WrapDB                |
| CPM         | Is a CMake script                 | Uses its own package manager      | Uses its own wraps              |

**Choice: CMake.** SFML, the largest dependency, is built with CMake, and CPM is a CMake script. The presets give the same configuration on every machine and in the pipeline.

**Price:** the CMake language. `cmake/YounityModule.cmake` takes about a hundred lines to declare a module.

## Package manager: CPM

|                    | [CPM](https://github.com/cpm-cmake/CPM.cmake) | [Conan](https://conan.io)                   | [vcpkg](https://vcpkg.io)                   |
|--------------------|-----------------------------------------------|---------------------------------------------|---------------------------------------------|
| What it is         | A CMake script, downloaded by the build       | A Python program with its recipes           | A repository of ports with a toolchain file |
| To install first   | CMake and Git                                 | Python and Conan                            | A clone of vcpkg, bootstrapped              |
| How libraries are built | From source, inside the build of the engine | Prebuilt when a binary matches, else source | From source, then cached as binaries     |
| Pinning a version  | A tag, or an archive with its SHA-256         | A version and a lockfile                    | A baseline commit of the ports              |

**Choice: CPM.** A fresh clone builds with CMake and Git alone. A game takes the engine the same way, with one `CPMAddPackage` call.

| Library       | Version | Pinned by                       |
|---------------|---------|---------------------------------|
| CPM           | 0.43.2  | SHA-256 of the script           |
| SFML          | 3.0.2   | SHA-256 of the release archive  |
| Asio          | 1.38.2  | Git tag                         |
| Dear ImGui    | 1.91.9b | SHA-256 of the release archive  |
| ImGui-SFML    | 3.0     | SHA-256 of the release archive  |
| nlohmann/json | 3.12.0  | SHA-256 of the release archive  |
| Lua           | 5.4.9   | SHA-256 of the release archive  |
| sol2          | Commit `c1f95a77` | SHA-256 of the commit archive |
| Criterion     | 2.4.3   | SHA-256 of the release archive, on Linux |

**Price:** SFML is compiled from source on each new machine. From an empty cache, an 8-core laptop took 35 seconds to configure and 14 seconds to compile.

## Scene files: JSON with nlohmann/json

The [`ecs`](../architecture/ecs.md#scene-files) module saves and loads scenes as JSON. The file is written by hand in a merge request and edited outside the engine, so it needs a readable format and a parser that refuses malformed input.

|                        | Hand-written parser              | [nlohmann/json](https://github.com/nlohmann/json) 3.12.0 (ours) | [RapidJSON](https://rapidjson.org)       |
|------------------------|----------------------------------|------------------------------------------------------------------|------------------------------------------|
| Values in the code     | Ours                             | `json`, a variant of object, array and scalar                    | `Value`, built with an allocator         |
| Reading and writing    | Two pieces to write and test     | One API for both                                                 | One API for both                         |
| Header-only            | Yes, no library                  | Yes                                                              | Yes                                      |
| Compile time           | Low                              | High: one large header                                           | Lower                                    |
| Malformed input        | Ours to detect                   | An exception, caught once in `loadScene`                          | A parse result to check                  |

**Choice: nlohmann/json.** Its values match the scene schema directly, the same types are read and written, and the parser is one include with no build step. `loadScene` catches the parser exception and returns `SceneFileError`; the schema and the range checks stay in the engine, where they know the registered components.

**Price:** the header is large and adds to the compile time of the ECS module. The parser builds the whole document in memory, like the rest of the loader, and the file still has no size limit. There is no schema library: every key, type and range is checked by hand, and version 1 has no migration. The dependency is pinned by the SHA-256 of its release archive.

## Window, graphics, audio and input: SFML

|                | [SFML 3](https://www.sfml-dev.org)         | [SDL 3](https://www.libsdl.org)                 | [Raylib](https://www.raylib.com)               |
|----------------|--------------------------------------------|-------------------------------------------------|------------------------------------------------|
| API            | C++17 classes that release their resources | C functions and handles                         | C functions, one global window                 |
| Scope          | Window, 2D graphics, text, audio, input    | Window, 2D renderer, audio output, input        | Window, 2D and 3D graphics, text, audio, input |
| Text and music | Included                                   | In SDL_ttf and SDL_mixer                        | Included                                       |
| Gamepads       | Buttons by number                          | Buttons by name, from a database of controllers | Buttons by name                                |

**Choice: SFML.** One C++ library covers what `render`, `input` and `audio` need, and its scope is 2D like the engine. Its network module is left out, the sockets coming from Asio, and a server that switches `render`, `input`, `audio` and `editor` off builds free of SFML.

**Price:** system headers on Linux (X11, OpenGL, udev), and gamepad buttons known by number, which the rebindable actions of the input module cover.

## Scripting: Lua 5.4 with sol2

A script gives an entity its behaviour from a file, without compiling the game again. The language runs inside the engine, so it has to be small, built with the engine, and cheap to call every frame.

|                       | [Lua 5.4](https://www.lua.org)                  | [Python](https://docs.python.org/3/extending/embedding.html) | C++ only               |
|-----------------------|-------------------------------------------------|---------------------------------------------------------------|------------------------|
| Made to be embedded   | Yes, it is its main use                         | Possible, the interpreter is made to run on its own           | Nothing to embed       |
| What the engine ships | 32 C files, built from source with the engine   | The interpreter and its standard library                      | Nothing                |
| Change a behaviour    | Edit the file                                   | Edit the file                                                 | Compile the game again |
| License               | MIT                                             | PSF                                                           |                        |

The C API of Lua works on a stack of values: every call is a sequence of pushes, a call and pops. A binding library writes that code from the C++ types.

|                 | Lua C API                          | [sol2](https://github.com/ThePhD/sol2) (ours)   | [LuaBridge3](https://github.com/kunitoki/LuaBridge3) |
|-----------------|------------------------------------|-------------------------------------------------|------------------------------------------------------|
| Bind a C++ type | Stack calls and metatables by hand | One call per type, its members listed           | One call per type, its members listed                |
| Header-only     | Nothing to add                     | Yes                                             | Yes                                                  |
| Compile time    | Low: 0.5 s for a file              | High: 6 s for a file that runs one script       | Not measured                                         |
| Last release    | Comes with Lua                     | 3.3.0, 2022, which fails to build on Clang      | Release candidates only, 3.0-rc13                    |
| License         | MIT                                | MIT                                             | Not detected as a standard license                   |

**Choice: Lua 5.4.9 with sol2.** Lua is made to be embedded and builds from source with the rest of the engine. sol2 binds a component in one call, where the C API needs the stack code of every field written and tested by hand. Lua is pinned by the SHA-256 of its release archive, the one published on lua.org; sol2 by a commit of its main branch, `c1f95a77` from March 2025, which builds with Clang.

**Price:** sol2 has had no release since 2022, and its last one fails to build on a recent Clang (`no member named 'construct' in 'sol::optional<T &>'`), so the engine depends on an unreleased commit. sol2 does not support Lua 5.5, which keeps the engine on 5.4. A file that includes sol2 takes 6 s to compile instead of 0.5 s (Clang, Intel Core i9-9880H laptop), so only the sources of the `scripting` module include it.

## Sockets: Asio

|                   | System sockets                        | [Asio](https://think-async.com/Asio/) | SFML network                          | [ENet](http://enet.bespin.org)       |
|-------------------|---------------------------------------|---------------------------------------|---------------------------------------|--------------------------------------|
| Linux and Windows | Two implementations to write and test | One API                               | One API                               | One API                              |
| Brings            | Sockets                               | Sockets, timers, asynchronous I/O     | Sockets and a selector                | Its own reliable protocol over UDP   |
| Dependency        | The system                            | Headers only                          | The SFML libraries, on the server too | A small C library                    |
| Wire format       | Ours                                  | Ours                                  | Ours                                  | The header of ENet in every datagram |

**Choice: Asio**, standalone, behind `network::UdpSocket`. One code for Linux and Windows, and a wire format that stays ours. `UdpPump` runs the socket on two threads of its own, so the game loop is free of network waits.

**Price:** a large library for what the engine uses, blocking calls on IPv4. Its asynchronous part is left unused.

## Transport: UDP

|               | UDP                                        | TCP                                           |
|---------------|--------------------------------------------|-----------------------------------------------|
| Unit          | A datagram, which arrives whole or is lost | A stream of bytes, to cut into messages       |
| A lost packet | Stays lost                                 | Is sent again, and everything behind it waits |
| Order         | As the network delivers                    | Kept                                          |
| Connection    | Ours to build                              | Built in                                      |

**Choice: UDP.** A position that arrives late has already been replaced by a newer one, and TCP would hold every packet behind a lost one.

**Price:** sessions, ordering and resending are ours to build. Each packet carries a sequence number for that.

TCP would fit exchanges outside the simulation, like downloading a level or a chat.

## Unit tests: Criterion

|                   | [Criterion](https://github.com/Snaipe/Criterion) | [GoogleTest](https://github.com/google/googletest) | [Catch2](https://github.com/catchorg/Catch2) |
|-------------------|--------------------------------------------------|----------------------------------------------------|----------------------------------------------|
| Isolation         | Each test in its own process                     | All the tests in one process                       | All the tests in one process                 |
| A crash in a test | A failure of that test, the others keep running  | Ends the run                                       | Ends the run                                 |
| How to get it     | A prebuilt archive for Linux x86_64              | Built from source with CMake                       | Built from source with CMake                 |
| Windows           | To build by hand, with Meson                     | Yes                                                | Yes                                          |

**Choice: Criterion.** Each test runs in its own process, so a crash is one failed test and the others keep running. The engine declares 747 test cases.

**Price:** the tests run on Linux, and on macOS with Criterion from Homebrew. On Windows the pipeline builds the engine and stops there. It is the choice we would revisit first, since Catch2 and GoogleTest build on the three systems.

## Formatter, linter and warnings

| Tool              | What it checks                                                                  | When                                  |
|-------------------|---------------------------------------------------------------------------------|---------------------------------------|
| clang-format      | The layout: 4 spaces, braces, spacing                                           | `format` job, any difference fails    |
| clang-tidy        | Names, braces around every statement, `nullptr`, `override`, `std::make_unique`, and the static analyzer of Clang | `tidy` job, every warning is an error |
| Compiler warnings | `-Wall -Wextra -Wpedantic`, or `/W4` on Windows                                 | Every build                           |

**Choice: the LLVM tools**, the ones editors run through clangd. A contributor sees in the editor what the pipeline will say. Cppcheck would fit as one more job beside `tidy`.

**Price:** `.clang-tidy` adds seven checks of style and modern idioms to the default set of clang-tidy, the static analyzer. The `bugprone` family is the next one to switch on.

## Continuous integration: GitLab CI

| Stage    | Jobs                           | What they do                                                  |
|----------|--------------------------------|---------------------------------------------------------------|
| `check`  | `format`, `tidy`, `docs`       | Format, static analysis, build of this website in strict mode |
| `build`  | `build-linux`, `build-windows` | Build with Clang 20, and with clang-cl through Visual Studio  |
| `test`   | `tests-linux`                  | Unit tests and coverage                                       |
| `deploy` | `pages`                        | Publication of this website, from `main`                      |

**Choice: GitLab CI.** The repository, the merge requests, the pipeline and this website sit in one tool. GitHub Actions would need a mirror of the repository, and Jenkins one more server.

A merge request pipeline takes four to six minutes. The build and test jobs run side by side, and the cache of a branch keeps the sources of the dependencies, with the compiled objects for the Linux build.

**Price:** the instance and the two runners of the pipeline are ours to keep running.

## Documentation: MkDocs and Doxygen

|               | MkDocs Material with MkDoxy                    | Doxygen alone                      | Sphinx with Breathe              | GitLab wiki              |
|---------------|------------------------------------------------|------------------------------------|----------------------------------|--------------------------|
| Written pages | Markdown                                       | Markdown, in the layout of Doxygen | reStructuredText or Markdown     | Markdown                 |
| API reference | From the Doxygen comments, in the same website | Its main job                       | From the Doxygen comments        | Written by hand          |
| Review        | In the merge request, with the code            | In the merge request               | In the merge request             | In a separate repository |
| To install    | Python, three packages, Doxygen                | Doxygen                            | Python, Sphinx, Breathe, Doxygen | A browser                |

**Choice: [MkDocs](https://www.mkdocs.org) with the [Material](https://squidfunk.github.io/mkdocs-material/) theme, and [MkDoxy](https://github.com/JakubAndrysek/MkDoxy) for the reference.** The pages are Markdown beside the code, reviewed in the same merge request, and the reference generated from the header comments lands in the same website. The pipeline builds it in strict mode, so a broken link fails the merge request.

**Price:** Python and Doxygen to read the website locally.
