# Technical study

This section explains the technical choices of the engine. Each choice is set beside its alternatives, with the reason it won and the price it has.

## What the choices answer to

A few requirements were fixed from the start, and most choices follow from them.

- The engine is written in C++ and runs on Linux and Windows.
- It builds from a fresh clone: the build brings the third-party libraries, and the machine provides the compiler and the platform libraries.
- It serves a networked game, where a dedicated server holds the game logic and talks to its clients over UDP with a binary protocol.
- Rendering, networking and game logic are separate subsystems, and the engine is reused from one game to the next.
- Three people build it in six weeks.

## How to read it

Each topic follows the same order: a table, the choice and its reason, then what the choice costs.

A number in these pages is either measured on the engine, with the method given at the [end of the algorithms page](algorithms.md#how-the-numbers-were-measured), or read from the pipeline of the project.

## The pages

| Page                                              | What it covers                                                                  |
|---------------------------------------------------|---------------------------------------------------------------------------------|
| [Technologies](technologies.md)                   | Language standard, build system, package manager, libraries, tests, tools       |
| [Algorithms and data structures](algorithms.md)   | Architecture, component storage, events, collisions, time, serialization        |
| [Storage](storage.md)                             | Resource cache, logs, configuration, what is not stored                         |
| [Security](security.md)                           | Trust boundaries, packet checks, fuzzing, asset paths, dependencies, gaps       |

## The choices at a glance

| Topic                          | Choice                          | Compared with                              |
|--------------------------------|---------------------------------|--------------------------------------------|
| Language standard              | C++23                           | C++17, C++20                               |
| Build system                   | CMake                           | Xmake, Meson                               |
| Package manager                | CPM                             | Conan, vcpkg                               |
| Window, graphics, audio, input | SFML 3                          | SDL 3, Raylib                              |
| Sockets                        | Asio                            | System sockets, SFML network, ENet         |
| Transport                      | UDP                             | TCP                                        |
| Unit tests                     | Criterion                       | GoogleTest, Catch2                         |
| Formatter and linter           | clang-format, clang-tidy        | Cppcheck                                   |
| Continuous integration         | GitLab CI                       | GitHub Actions, Jenkins                    |
| Documentation                  | MkDocs with Doxygen             | Doxygen alone, Sphinx, a wiki              |
| Architecture                   | Entity-component-system         | Inheritance tree, objects with components  |
| Component storage              | Sparse arrays                   | Sparse sets, archetypes                    |
| Systems and entities           | Signatures                      | A query at each frame, archetypes          |
| Communication between modules  | Event bus                       | Direct calls, an event queue               |
| Collision detection            | Every pair tested               | Grid, quadtree, sweep and prune            |
| Time                           | Fixed steps                     | Variable steps                             |
| Serialization                  | Our own binary writer and reader | Raw structs, text, a schema library       |
| Resources                      | On-demand cache                 | Preload everything, stream on each use     |
| Logs                           | Console and optional file       | System log, rotating file                  |
| Configuration                  | Arguments and environment       | Configuration file, settings service       |
| Malformed packets              | Rejected by the envelope        | Trusted sizes, no check                    |
| Asset paths                    | Lexical confinement             | Canonical path, container                  |
| Dependencies                   | Version pins and hashes         | Floating versions, system packages         |
