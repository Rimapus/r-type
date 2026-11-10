# Use it in your game

A game gets the engine with [CPM](https://github.com/cpm-cmake/CPM.cmake) and links the modules it needs. Each module is a CMake target named `Younity::<Module>`.

## Add the engine to your project

```cmake
cmake_minimum_required(VERSION 3.21)
project(MyGame LANGUAGES CXX)

include(cmake/CPM.cmake)

CPMAddPackage(
    NAME Younity
    SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../younity-engine"
)

add_executable(my_game main.cpp)
target_link_libraries(my_game PRIVATE Younity::Core Younity::Render)
```

Outside this repository, replace `SOURCE_DIR` with `GIT_REPOSITORY` and a `GIT_TAG`.

Linking a module also gives its headers and the C++23 standard to your target.

## Open a window

```cpp
#include "younity/render/Window.hpp"

int main(void)
{
    younity::render::Window window(800, 600, "My game");

    while (window.isOpen()) {
        window.pollEvents();
        window.clear();
        window.display();
    }
    return 0;
}
```

This opens a window and keeps it alive until it is closed.

## Load a plugin

A game can load shared libraries at run time with the `plugin` module. The game keeps a `younity::plugin::PluginManager`, loads its JSON configuration in `onStart` and updates the plugins once per step:

```cpp
void onStart(void) override
{
    const std::expected<void, younity::plugin::PluginError> loaded = this->_plugins.loadFromFile("plugins.json");

    if (loaded) {
        this->_plugins.start();
    }
}

void onFixedUpdate(float step) override
{
    this->_plugins.update(step);
}
```

The configuration lists the libraries, and each one exports the descriptor of the plugin. Build them with `younity_add_plugin`: [Plugin](../architecture/plugin.md) covers the descriptor, the configuration and the lifecycle.

## The targets

| Target               | Headers                 |
|----------------------|-------------------------|
| `Younity::Logger`    | `younity/logger/...`    |
| `Younity::Core`      | `younity/core/...`      |
| `Younity::Tools`     | `younity/tools/...`     |
| `Younity::Plugin`    | `younity/plugin/...`    |
| `Younity::Ecs`       | `younity/ecs/...`       |
| `Younity::Scripting` | `younity/scripting/...` |
| `Younity::Physics`   | `younity/physics/...`   |
| `Younity::Assets`    | `younity/assets/...`    |
| `Younity::Render`    | `younity/render/...`    |
| `Younity::Input`     | `younity/input/...`     |
| `Younity::Audio`     | `younity/audio/...`     |
| `Younity::Network`   | `younity/network/...`   |
| `Younity::Editor`    | `younity/editor/...`    |

## Next steps

- [Your first entity and system](../guides/first-entity.md)
- [API reference](../api/annotated.md)
