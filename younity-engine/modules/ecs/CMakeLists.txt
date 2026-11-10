include(${PROJECT_SOURCE_DIR}/cmake/JSON.cmake)

younity_add_module(ecs DEPENDS core LIBRARIES nlohmann_json::nlohmann_json)
