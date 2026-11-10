# jtype_add_library(<name> [LIBRARIES <library>...])
#
# Creates the jtype_<name> target and its JType::<Name> alias from the calling folder:
#   include/jtype/<name>/   public headers
#   src/                    sources, globbed recursively
# A library without any .cpp yet (or header-only) is an INTERFACE library,
# it becomes a static library as soon as a .cpp is added in src/.
# LIBRARIES lists the libraries it is built on (e.g. Younity::Ecs): its users get them too.
function(jtype_add_library name)
    cmake_parse_arguments(PARSE_ARGV 1 ARG "" "" "LIBRARIES")

    set(target jtype_${name})
    string(SUBSTRING ${name} 0 1 first_letter)
    string(SUBSTRING ${name} 1 -1 other_letters)
    string(TOUPPER ${first_letter} first_letter)
    set(alias JType::${first_letter}${other_letters})

    file(GLOB_RECURSE sources CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

    if(sources)
        add_library(${target} STATIC ${sources})
        set(scope PUBLIC)
    else()
        add_library(${target} INTERFACE)
        set(scope INTERFACE)
    endif()
    add_library(${alias} ALIAS ${target})

    target_include_directories(${target} ${scope} ${CMAKE_CURRENT_SOURCE_DIR}/include)
    target_link_libraries(${target} ${scope} ${ARG_LIBRARIES})
    jtype_set_coverage(${target} ${scope})

    if(sources)
        jtype_set_warnings(${target})
    endif()
endfunction()

# jtype_add_binary(<name> [LIBRARIES <library>...])
#
# Creates the r-type_<name> binary, the name asked by the subject, from the src/ folder of the calling folder.
# It is written at the root of the repository and installed at the root of the package,
# so it is next to the assets folder in both cases.
function(jtype_add_binary name)
    cmake_parse_arguments(PARSE_ARGV 1 ARG "" "" "LIBRARIES")

    set(target r-type_${name})

    file(GLOB_RECURSE sources CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

    add_executable(${target} ${sources})
    target_link_libraries(${target} PRIVATE ${ARG_LIBRARIES})
    jtype_set_warnings(${target})
    jtype_set_coverage(${target} PRIVATE)

    set_target_properties(${target} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}"
        RUNTIME_OUTPUT_DIRECTORY_DEBUG "${PROJECT_SOURCE_DIR}"
        RUNTIME_OUTPUT_DIRECTORY_RELEASE "${PROJECT_SOURCE_DIR}"
        RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${PROJECT_SOURCE_DIR}"
        RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${PROJECT_SOURCE_DIR}"
    )

    install(TARGETS ${target} RUNTIME DESTINATION . COMPONENT game)
endfunction()

function(jtype_set_warnings target)
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4)
    else()
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic)
    endif()
endfunction()

# gcovr instrumentation, enabled by the "coverage" preset
function(jtype_set_coverage target scope)
    if(JTYPE_COVERAGE AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(${target} ${scope} --coverage)
        target_link_options(${target} ${scope} --coverage)
    endif()
endfunction()
