include(${PROJECT_SOURCE_DIR}/cmake/Criterion.cmake)

file(GLOB_RECURSE J_CLIMB_TESTS_SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

add_executable(j-climb_tests ${J_CLIMB_TESTS_SOURCES})

target_link_libraries(j-climb_tests PRIVATE criterion::criterion jclimb_world)
# The assets folder, with the real scripts, and the test rooms, whatever folder the tests run from
target_compile_definitions(j-climb_tests PRIVATE
    JCLIMB_ASSETS_DIR="${PROJECT_SOURCE_DIR}/assets"
    JCLIMB_TESTS_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
)
jclimb_set_warnings(j-climb_tests)

add_custom_target(tests_run
    COMMAND j-climb_tests
    DEPENDS j-climb_tests
    USES_TERMINAL
)
