# Provides the asio::asio target (standalone Asio, header-only), fetched with CPM.
# Asio itself has no CMake build: the target only carries the include directory,
# the ASIO_STANDALONE define and the platform link dependencies (threads, Winsock).

include_guard(GLOBAL)
include(${CMAKE_CURRENT_LIST_DIR}/CPM.cmake)

CPMAddPackage(
    NAME asio
    GITHUB_REPOSITORY chriskohlhoff/asio
    GIT_TAG asio-1-38-2
)

if(NOT TARGET asio::asio)
    add_library(asio INTERFACE)
    add_library(asio::asio ALIAS asio)

    target_include_directories(asio SYSTEM INTERFACE ${asio_SOURCE_DIR}/include)
    target_compile_definitions(asio INTERFACE ASIO_STANDALONE)

    find_package(Threads REQUIRED)
    target_link_libraries(asio INTERFACE Threads::Threads)

    if(WIN32)
        target_link_libraries(asio INTERFACE ws2_32)
    endif()
endif()
