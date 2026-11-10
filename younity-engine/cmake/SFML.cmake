# Provides the SFML::Graphics, SFML::Window and SFML::Audio targets, fetched with CPM and linked statically.
# SFML builds its own dependencies (FreeType, Ogg, Vorbis, FLAC) instead of using the system ones,
# only platform libraries are needed: X11, OpenGL and udev on Linux.

include_guard(GLOBAL)
include(${CMAKE_CURRENT_LIST_DIR}/CPM.cmake)

# On macOS, SFML has Objective-C sources that only clang can compile against Apple's headers,
# so use it unless a C compiler was chosen: the default "cc" can be a GCC (e.g. from Homebrew)
if(APPLE AND NOT CMAKE_C_COMPILER AND NOT DEFINED ENV{CC})
    set(ENV{CC} clang)
endif()

# Keep SFML and its dependencies out of compile_commands.json: clangd borrows the flags of the .cpp
# with the closest name for a header, and SFML files (e.g. Vector2.cpp) would shadow the engine ones
set(CMAKE_EXPORT_COMPILE_COMMANDS OFF)

CPMAddPackage(
    NAME SFML
    VERSION 3.0.2
    URL https://github.com/SFML/SFML/archive/refs/tags/3.0.2.tar.gz
    URL_HASH SHA256=0034e05f95509e5d3fb81b1625713e06da7b068f210288ce3fd67106f8f46995
    SYSTEM YES
    OPTIONS
        "SFML_BUILD_NETWORK OFF"
        "SFML_USE_SYSTEM_DEPS OFF"
)

# Back to the preset value for the engine targets
unset(CMAKE_EXPORT_COMPILE_COMMANDS)

# SFML is C++17 code that misses some includes in C++23 with libc++ (macOS):
# keep it in C++17 even when the parent project sets CMAKE_CXX_STANDARD (like j-type does)
foreach(sfml_target IN ITEMS sfml-system sfml-window sfml-graphics sfml-audio sfml-main)
    if(TARGET ${sfml_target})
        set_target_properties(${sfml_target} PROPERTIES CXX_STANDARD 17)
    endif()
endforeach()
