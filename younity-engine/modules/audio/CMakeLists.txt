include(${PROJECT_SOURCE_DIR}/cmake/SFML.cmake)

younity_add_module(audio DEPENDS core assets LIBRARIES SFML::Audio)
