include(${PROJECT_SOURCE_DIR}/cmake/ImGui.cmake)

younity_add_module(editor DEPENDS core ecs assets render input tools LIBRARIES ImGui-SFML::ImGui-SFML)
