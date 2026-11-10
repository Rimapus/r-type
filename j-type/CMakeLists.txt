cmake_minimum_required(VERSION 3.21)

project(JType
    VERSION 0.4.1
    DESCRIPTION "J-Type, built on the Younity engine"
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

# J-Climb, the second game built on the engine: the Konami code on the main menu starts it
CPMAddPackage(
    NAME JClimb
    SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../j-climb"
)

include(cmake/JTypeTarget.cmake)

# The code shared by both sides first, then the two binaries that use it
add_subdirectory(common)
add_subdirectory(server)
add_subdirectory(client)
include(cmake/JClimb.cmake)

# Criterion unit tests, enabled by the "tests" preset
option(JTYPE_BUILD_TESTS "Build the J-Type unit tests" OFF)

# gcovr coverage instrumentation, enabled by the "coverage" preset
option(JTYPE_COVERAGE "Instrument the J-Type targets for coverage" OFF)

if(JTYPE_BUILD_TESTS)
    add_subdirectory(tests)
endif()

# LibFuzzer harnesses, enabled by the "fuzz" preset (needs Clang)
option(JTYPE_BUILD_FUZZERS "Build the J-Type fuzzing harnesses" OFF)

if(JTYPE_BUILD_FUZZERS)
    add_subdirectory(fuzz)
endif()

# Package of the game, built by the "release" preset: the binaries next to the assets folder
install(DIRECTORY assets DESTINATION . COMPONENT game)
include(cmake/Package.cmake)
