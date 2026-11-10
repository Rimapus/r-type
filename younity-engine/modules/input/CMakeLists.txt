include(${PROJECT_SOURCE_DIR}/cmake/SFML.cmake)

younity_add_module(input DEPENDS core LIBRARIES SFML::Window)
