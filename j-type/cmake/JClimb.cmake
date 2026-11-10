# J-Climb goes next to the client, in its own folder with its assets, where the main menu starts it from:
#   j-climb/j-climb        the game, j-climb.exe on Windows
#   j-climb/assets/        its scripts, sprites, sounds, music and scene
# It is built with the client, and packaged with the game.

set(JTYPE_JCLIMB_DIR "${PROJECT_SOURCE_DIR}/j-climb")

set_target_properties(j-climb PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${JTYPE_JCLIMB_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${JTYPE_JCLIMB_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${JTYPE_JCLIMB_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${JTYPE_JCLIMB_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${JTYPE_JCLIMB_DIR}"
)
# Copied at each build, so a script changed in J-Climb reaches the copy
add_custom_target(j-climb_assets
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${JClimb_SOURCE_DIR}/assets" "${JTYPE_JCLIMB_DIR}/assets"
)
add_dependencies(r-type_client j-climb j-climb_assets)

install(TARGETS j-climb RUNTIME DESTINATION j-climb COMPONENT game)
install(DIRECTORY "${JClimb_SOURCE_DIR}/assets" DESTINATION j-climb COMPONENT game)
