cmake_minimum_required(VERSION 3.21)

project(Younity
    VERSION 0.13.0
    DESCRIPTION "Younity game engine"
    LANGUAGES CXX
)

include(cmake/YounityModule.cmake)

# Mandatory modules: the logger comes first and depends on nothing, so every other module can log
add_subdirectory(modules/logger)
add_subdirectory(modules/core)

# Optional modules, listed in dependency order: a module can only depend on the ones before it
set(YOUNITY_OPTIONAL_MODULES tools plugin ecs physics scripting assets render input audio network editor)

foreach(module IN LISTS YOUNITY_OPTIONAL_MODULES)
    string(TOUPPER ${module} module_option)
    option(YOUNITY_BUILD_${module_option} "Build the Younity ${module} module" ON)
    if(YOUNITY_BUILD_${module_option})
        add_subdirectory(modules/${module})
    endif()
endforeach()

# Criterion unit tests, enabled by the "tests" preset
option(YOUNITY_BUILD_TESTS "Build the Younity unit tests" OFF)

# gcovr coverage instrumentation, enabled by the "coverage" preset
option(YOUNITY_COVERAGE "Instrument the Younity targets for coverage" OFF)

if(YOUNITY_BUILD_TESTS)
    add_subdirectory(tests)
endif()

# LibFuzzer harnesses, enabled by the "fuzz" preset (needs Clang)
option(YOUNITY_BUILD_FUZZERS "Build the Younity fuzzing harnesses" OFF)

if(YOUNITY_BUILD_FUZZERS)
    add_subdirectory(fuzz)
endif()
