include(${PROJECT_SOURCE_DIR}/cmake/JSON.cmake)
include(${PROJECT_SOURCE_DIR}/cmake/Lua.cmake)

younity_add_module(scripting DEPENDS core ecs physics LIBRARIES sol2::sol2 nlohmann_json::nlohmann_json)
