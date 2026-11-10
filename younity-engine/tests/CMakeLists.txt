include(${PROJECT_SOURCE_DIR}/cmake/Criterion.cmake)

# One binary for all the tests: tests/<module>/ is compiled only if the module is enabled
add_executable(younity_tests)
target_link_libraries(younity_tests PRIVATE criterion::criterion)
set_target_properties(younity_tests PROPERTIES CXX_EXTENSIONS OFF)
younity_set_warnings(younity_tests)

# Folder of the test files, e.g. the images of tests/render/fixtures/, whatever folder the tests run from
target_compile_definitions(younity_tests PRIVATE YOUNITY_TESTS_DIR="${CMAKE_CURRENT_SOURCE_DIR}")

if(YOUNITY_COVERAGE AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(younity_tests PRIVATE --coverage)
    target_link_options(younity_tests PRIVATE --coverage)
endif()

foreach(module IN ITEMS logger core ${YOUNITY_OPTIONAL_MODULES})
    if(TARGET younity_${module})
        file(GLOB_RECURSE module_tests CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${module}/*.cpp)
        # A fixture is a source built as a plugin, not a test of the module
        list(FILTER module_tests EXCLUDE REGEX "/fixtures/")
        target_sources(younity_tests PRIVATE ${module_tests})
        target_link_libraries(younity_tests PRIVATE younity_${module})
    endif()
endforeach()

# The debug window tests drive Dear ImGui themselves, without a window
if(TARGET ImGui-SFML::ImGui-SFML)
    target_link_libraries(younity_tests PRIVATE ImGui-SFML::ImGui-SFML)
endif()

# The plugin tests load shared libraries built from the fixtures
if(TARGET younity_plugin)
    set(plugin_fixtures ${CMAKE_CURRENT_SOURCE_DIR}/plugin/fixtures)

    younity_add_plugin(younity_test_plugin_valid SOURCES ${plugin_fixtures}/valid.cpp DEPENDS plugin core)
    younity_add_plugin(younity_test_plugin_first SOURCES ${plugin_fixtures}/valid.cpp DEPENDS plugin core)
    target_compile_definitions(younity_test_plugin_first PRIVATE YOUNITY_TEST_PLUGIN_NAME="first")
    younity_add_plugin(younity_test_plugin_second SOURCES ${plugin_fixtures}/valid.cpp DEPENDS plugin core)
    target_compile_definitions(younity_test_plugin_second PRIVATE
        YOUNITY_TEST_PLUGIN_NAME="second"
        YOUNITY_TEST_PLUGIN_VERSION="2.0.0"
    )
    younity_add_plugin(younity_test_plugin_missing_descriptor
        SOURCES ${plugin_fixtures}/missing_descriptor.cpp DEPENDS plugin core)
    younity_add_plugin(younity_test_plugin_bad_abi SOURCES ${plugin_fixtures}/bad_abi.cpp DEPENDS plugin core)
    younity_add_plugin(younity_test_plugin_null_descriptor
        SOURCES ${plugin_fixtures}/null_descriptor.cpp DEPENDS plugin core)
    younity_add_plugin(younity_test_plugin_invalid_descriptor
        SOURCES ${plugin_fixtures}/invalid_descriptor.cpp DEPENDS plugin core)
    younity_add_plugin(younity_test_plugin_stateless
        SOURCES ${plugin_fixtures}/stateless.cpp DEPENDS plugin core)

    target_compile_definitions(younity_tests PRIVATE
        YOUNITY_TEST_PLUGIN_VALID="$<TARGET_FILE:younity_test_plugin_valid>"
        YOUNITY_TEST_PLUGIN_FIRST="$<TARGET_FILE:younity_test_plugin_first>"
        YOUNITY_TEST_PLUGIN_SECOND="$<TARGET_FILE:younity_test_plugin_second>"
        YOUNITY_TEST_PLUGIN_MISSING_DESCRIPTOR="$<TARGET_FILE:younity_test_plugin_missing_descriptor>"
        YOUNITY_TEST_PLUGIN_BAD_ABI="$<TARGET_FILE:younity_test_plugin_bad_abi>"
        YOUNITY_TEST_PLUGIN_NULL_DESCRIPTOR="$<TARGET_FILE:younity_test_plugin_null_descriptor>"
        YOUNITY_TEST_PLUGIN_INVALID_DESCRIPTOR="$<TARGET_FILE:younity_test_plugin_invalid_descriptor>"
        YOUNITY_TEST_PLUGIN_STATELESS="$<TARGET_FILE:younity_test_plugin_stateless>"
    )
    add_dependencies(younity_tests
        younity_test_plugin_valid
        younity_test_plugin_first
        younity_test_plugin_second
        younity_test_plugin_missing_descriptor
        younity_test_plugin_bad_abi
        younity_test_plugin_null_descriptor
        younity_test_plugin_invalid_descriptor
        younity_test_plugin_stateless
    )
endif()

add_custom_target(tests_run
    COMMAND younity_tests
    DEPENDS younity_tests
    USES_TERMINAL
)
