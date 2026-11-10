include(${PROJECT_SOURCE_DIR}/cmake/SFML.cmake)
include(${PROJECT_SOURCE_DIR}/cmake/ImGui.cmake)

younity_add_module(render DEPENDS core ecs assets tools LIBRARIES SFML::Graphics ImGui-SFML::ImGui-SFML)
