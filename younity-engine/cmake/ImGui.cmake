# Provides the ImGui-SFML::ImGui-SFML target: Dear ImGui and its SFML backend, fetched with CPM and linked statically.
# Dear ImGui has no CMake build: ImGui-SFML compiles its sources from the folder given in IMGUI_DIR.
# ImGui-SFML 3.0 is the release for SFML 3 and is tested with the Dear ImGui 1.91 series.

include_guard(GLOBAL)
include(${CMAKE_CURRENT_LIST_DIR}/CPM.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/SFML.cmake)

# Kept out of compile_commands.json, like SFML: clangd would borrow their flags for the engine headers
set(CMAKE_EXPORT_COMPILE_COMMANDS OFF)

CPMAddPackage(
    NAME imgui
    VERSION 1.91.9b
    URL https://github.com/ocornut/imgui/archive/refs/tags/v1.91.9b.tar.gz
    URL_HASH SHA256=8e1bbc76c71d74fef2fb85db7e7ca8eba13d6a86623c54992b60162db554ffdb
    DOWNLOAD_ONLY YES
)

CPMAddPackage(
    NAME ImGui-SFML
    VERSION 3.0
    URL https://github.com/SFML/imgui-sfml/archive/refs/tags/v3.0.tar.gz
    URL_HASH SHA256=561a04407dcc913b8f1c25a164ad4907852b51fb4f11907d1ab9db6351af911b
    SYSTEM YES
    OPTIONS
        "IMGUI_DIR ${imgui_SOURCE_DIR}"
        # The SFML targets come from SFML.cmake
        "IMGUI_SFML_FIND_SFML OFF"
)

unset(CMAKE_EXPORT_COMPILE_COMMANDS)
