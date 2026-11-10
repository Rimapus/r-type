include(${PROJECT_SOURCE_DIR}/cmake/Asio.cmake)

younity_add_module(network DEPENDS core tools LIBRARIES asio::asio)
