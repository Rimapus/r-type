include(${PROJECT_SOURCE_DIR}/cmake/Criterion.cmake)

file(GLOB_RECURSE J_TYPE_TESTS_SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

# The parts of the client that draw nothing, so they are tested without a window
set(J_TYPE_TESTED_CLIENT_SOURCES
    ${PROJECT_SOURCE_DIR}/client/src/audio/SoundCues.cpp
    ${PROJECT_SOURCE_DIR}/client/src/input/Bindings.cpp
    ${PROJECT_SOURCE_DIR}/client/src/input/Controls.cpp
    ${PROJECT_SOURCE_DIR}/client/src/network/Connection.cpp
    ${PROJECT_SOURCE_DIR}/client/src/network/SnapshotAssembler.cpp
    ${PROJECT_SOURCE_DIR}/client/src/render/BitmapFont.cpp
    ${PROJECT_SOURCE_DIR}/client/src/render/Flashes.cpp
    ${PROJECT_SOURCE_DIR}/client/src/render/HudLayout.cpp
    ${PROJECT_SOURCE_DIR}/client/src/render/PlayerMarker.cpp
    ${PROJECT_SOURCE_DIR}/client/src/scenes/Captions.cpp
    ${PROJECT_SOURCE_DIR}/client/src/scenes/Help.cpp
    ${PROJECT_SOURCE_DIR}/client/src/scenes/KonamiCode.cpp
    ${PROJECT_SOURCE_DIR}/client/src/scenes/Menu.cpp
    ${PROJECT_SOURCE_DIR}/client/src/scenes/Notices.cpp
    ${PROJECT_SOURCE_DIR}/client/src/scenes/Popups.cpp
    ${PROJECT_SOURCE_DIR}/client/src/settings/Colorblind.cpp
    ${PROJECT_SOURCE_DIR}/client/src/settings/Settings.cpp
    ${PROJECT_SOURCE_DIR}/client/src/system/JClimb.cpp
    ${PROJECT_SOURCE_DIR}/client/src/system/Process.cpp
    ${PROJECT_SOURCE_DIR}/client/src/world/EventBuffer.cpp
    ${PROJECT_SOURCE_DIR}/client/src/world/ShipPrediction.cpp
    ${PROJECT_SOURCE_DIR}/client/src/world/SnapshotBuffer.cpp
)

# The parts of the server that open no socket, so they are tested without a port
set(J_TYPE_TESTED_SERVER_SOURCES
    ${PROJECT_SOURCE_DIR}/server/src/Commands.cpp
    ${PROJECT_SOURCE_DIR}/server/src/ConsoleInput.cpp
    ${PROJECT_SOURCE_DIR}/server/src/DropLog.cpp
    ${PROJECT_SOURCE_DIR}/server/src/EventTracker.cpp
    ${PROJECT_SOURCE_DIR}/server/src/InputGate.cpp
    ${PROJECT_SOURCE_DIR}/server/src/Signals.cpp
)

add_executable(j-type_tests ${J_TYPE_TESTS_SOURCES} ${J_TYPE_TESTED_CLIENT_SOURCES} ${J_TYPE_TESTED_SERVER_SOURCES})

# Input for the keys and the gamepad axes of the bindings
target_link_libraries(j-type_tests PRIVATE criterion::criterion JType::Common Younity::Input)
target_include_directories(j-type_tests PRIVATE ${PROJECT_SOURCE_DIR}/client/src ${PROJECT_SOURCE_DIR}/server/src)
# The folder of the project, e.g. to check the data files of assets/, whatever folder the tests run from
target_compile_definitions(j-type_tests PRIVATE JTYPE_SOURCE_DIR="${PROJECT_SOURCE_DIR}")
jtype_set_warnings(j-type_tests)
jtype_set_coverage(j-type_tests PRIVATE)

add_custom_target(tests_run
    COMMAND j-type_tests
    DEPENDS j-type_tests
    USES_TERMINAL
)
