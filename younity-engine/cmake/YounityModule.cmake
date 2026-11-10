# Folder of this file, where the Log.hpp.in template is: CMAKE_CURRENT_LIST_DIR would be the module folder in a function
set(YOUNITY_MODULE_CMAKE_DIR ${CMAKE_CURRENT_LIST_DIR})

# younity_add_module(<name> [DEPENDS <module>...] [LIBRARIES <library>...])
#
# Creates the younity_<name> target and its Younity::<Name> alias from the calling module folder:
#   include/younity/<name>/   public headers
#   src/                      sources, globbed recursively
# A module without any .cpp yet (or header-only) is an INTERFACE library,
# it becomes a compiled library as soon as a .cpp is added in src/.
# DEPENDS lists other Younity modules, LIBRARIES external ones (e.g. SFML::Graphics):
# external libraries are PRIVATE to a compiled module, so its users only see the Younity API.
# Every module but the logger itself gets a generated younity/<name>/Log.hpp header,
# giving younity::<name>::logger(): the logger named after the module.
function(younity_add_module name)
    cmake_parse_arguments(PARSE_ARGV 1 ARG "" "" "DEPENDS;LIBRARIES")

    set(target younity_${name})
    string(SUBSTRING ${name} 0 1 first_letter)
    string(SUBSTRING ${name} 1 -1 other_letters)
    string(TOUPPER ${first_letter} first_letter)
    set(alias Younity::${first_letter}${other_letters})

    file(GLOB_RECURSE sources CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

    if(sources)
        add_library(${target} ${sources})
        set(scope PUBLIC)
        set(libraries_scope PRIVATE)
    else()
        # A header-only module is compiled by its users, so they need its external libraries too
        add_library(${target} INTERFACE)
        set(scope INTERFACE)
        set(libraries_scope INTERFACE)
    endif()
    add_library(${alias} ALIAS ${target})

    target_include_directories(${target} ${scope} ${CMAKE_CURRENT_SOURCE_DIR}/include)
    target_compile_features(${target} ${scope} cxx_std_23)

    if(YOUNITY_COVERAGE AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(${target} ${scope} --coverage)
        target_link_options(${target} ${scope} --coverage)
    endif()

    if(NOT name STREQUAL "logger")
        younity_generate_log_header(${target} ${name} ${scope})
    endif()

    foreach(dependency IN LISTS ARG_DEPENDS)
        if(NOT TARGET younity_${dependency})
            string(TOUPPER ${dependency} dependency_option)
            message(FATAL_ERROR "Younity module '${name}' needs '${dependency}': set YOUNITY_BUILD_${dependency_option} to ON")
        endif()
        target_link_libraries(${target} ${scope} younity_${dependency})
    endforeach()

    if(ARG_LIBRARIES)
        target_link_libraries(${target} ${libraries_scope} ${ARG_LIBRARIES})
    endif()

    if(sources)
        # PIC: a plugin links the static engine modules into its shared library
        set_target_properties(${target} PROPERTIES CXX_EXTENSIONS OFF POSITION_INDEPENDENT_CODE ON)
        younity_set_warnings(${target})
    endif()
endfunction()

# younity_add_plugin(<name> SOURCES <source>... [DEPENDS <module>...] [LIBRARIES <library>...])
#
# Creates a shared library with no "lib" prefix, loaded at run time by younity::plugin::Plugin.
# The sources are explicit: a plugin is not an engine module and has no src/ convention.
# DEPENDS lists the Younity modules it links, e.g. core and plugin; LIBRARIES external ones.
function(younity_add_plugin name)
    cmake_parse_arguments(PARSE_ARGV 1 ARG "" "" "SOURCES;DEPENDS;LIBRARIES")

    if(NOT ARG_SOURCES)
        message(FATAL_ERROR "Younity plugin '${name}' has no source: pass SOURCES <source>...")
    endif()

    add_library(${name} MODULE ${ARG_SOURCES})
    set_target_properties(${name} PROPERTIES
        CXX_EXTENSIONS OFF
        PREFIX ""
        POSITION_INDEPENDENT_CODE ON
    )
    target_compile_features(${name} PRIVATE cxx_std_23)
    younity_set_warnings(${name})

    foreach(dependency IN LISTS ARG_DEPENDS)
        if(NOT TARGET younity_${dependency})
            string(TOUPPER ${dependency} dependency_option)
            message(FATAL_ERROR "Younity plugin '${name}' needs '${dependency}': set YOUNITY_BUILD_${dependency_option} to ON")
        endif()
        target_link_libraries(${name} PRIVATE younity_${dependency})
    endforeach()

    if(ARG_LIBRARIES)
        target_link_libraries(${name} PRIVATE ${ARG_LIBRARIES})
    endif()
endfunction()

# younity_generate_log_header(<target> <name> <scope>)
#
# Generates younity/<name>/Log.hpp from cmake/Log.hpp.in in the build folder, so it is never committed,
# and links the module to the logger it uses. The logger module must be added before any other module.
function(younity_generate_log_header target name scope)
    if(NOT TARGET younity_logger)
        message(FATAL_ERROR "Younity module '${name}' needs the logger: add modules/logger before it")
    endif()

    # Outside the module build folder, where the module is compiled from: Clang would record a header
    # generated there with a path relative to it in the coverage data, and gcovr could not find it
    set(generated_include_dir ${PROJECT_BINARY_DIR}/generated/${name}/include)

    set(YOUNITY_MODULE_NAME ${name})
    configure_file(
        ${YOUNITY_MODULE_CMAKE_DIR}/Log.hpp.in
        ${generated_include_dir}/younity/${name}/Log.hpp
        @ONLY
    )
    target_include_directories(${target} ${scope} ${generated_include_dir})
    target_link_libraries(${target} ${scope} younity_logger)
endfunction()

# younity_set_warnings(<target>)
#
# Turns on the warnings of the compiler for a target of the engine: /W4 with MSVC, -Wall -Wextra -Wpedantic otherwise.
function(younity_set_warnings target)
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4)
    else()
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic)
    endif()
endfunction()
