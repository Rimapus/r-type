include(${PROJECT_SOURCE_DIR}/cmake/JSON.cmake)

younity_add_module(plugin LIBRARIES nlohmann_json::nlohmann_json ${CMAKE_DL_LIBS})
