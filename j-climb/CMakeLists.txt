cmake_minimum_required(VERSION 3.21)

project(JClimb
    VERSION 0.3.1
    DESCRIPTION "J-Climb, a small precision platformer in Lua, built on the Younity engine"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(cmake/CPM.cmake)

CPMAddPackage(
    NAME Younity
    SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../younity-engine"
)

function(jclimb_set_warnings target)
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4)
    else()
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic)
    endif()
endfunction()

# The world runs the Lua scripts without a window, so the tests build it too
add_library(jclimb_world STATIC src/Controls.cpp src/World.cpp)
target_include_directories(jclimb_world PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(jclimb_world PUBLIC Younity::Ecs Younity::Physics Younity::Scripting Younity::Render Younity::Input)
jclimb_set_warnings(jclimb_world)

add_executable(j-climb src/Game.cpp src/main.cpp)
# Only the game plays sounds: the world and its tests give the scripts no audio
target_link_libraries(j-climb PRIVATE jclimb_world Younity::Audio)
jclimb_set_warnings(j-climb)

# Built alone, the game is written next to its assets folder, so ./j-climb finds it.
# Built inside another project, like J-Type, that project chooses where the game and its assets go.
if(PROJECT_IS_TOP_LEVEL)
    set_target_properties(j-climb PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}"
        RUNTIME_OUTPUT_DIRECTORY_DEBUG "${PROJECT_SOURCE_DIR}"
        RUNTIME_OUTPUT_DIRECTORY_RELEASE "${PROJECT_SOURCE_DIR}"
    )
    install(TARGETS j-climb RUNTIME DESTINATION . COMPONENT game)
    install(DIRECTORY assets DESTINATION . COMPONENT game)
    include(cmake/Package.cmake)
endif()

# Criterion unit tests, enabled by the "tests" preset
option(JCLIMB_BUILD_TESTS "Build the J-Climb unit tests" OFF)

if(JCLIMB_BUILD_TESTS)
    add_subdirectory(tests)
endif()
