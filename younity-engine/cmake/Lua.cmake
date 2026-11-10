# Provides the lua::lua target (the Lua 5.4 interpreter, built from its sources) and the sol2::sol2 target
# (header-only C++ binding of Lua), fetched with CPM. Both archives are pinned by their SHA-256.
# sol2 is pinned to a commit of its main branch: its last release, 3.3.0, fails to build on a recent Clang.

include_guard(GLOBAL)
include(${CMAKE_CURRENT_LIST_DIR}/CPM.cmake)

# The engine is a C++ project, Lua is written in C
enable_language(C)

CPMAddPackage(
    NAME lua
    VERSION 5.4.9
    URL https://www.lua.org/ftp/lua-5.4.9.tar.gz
    URL_HASH SHA256=2335b6c582a52654f94612bf10d2f4672805d05329aa6568b1d8cd9e5c6fb8e6
    DOWNLOAD_ONLY YES
)

CPMAddPackage(
    NAME sol2
    VERSION 3.3.0.20250307
    URL https://github.com/ThePhD/sol2/archive/c1f95a773c6f8f4fde8ca3efe872e7286afe4444.tar.gz
    URL_HASH SHA256=f3b7bff03c260c74c74bd55f812f7e174d5c357f576430ffaf12a206462d6356
    DOWNLOAD_ONLY YES
)

if(NOT TARGET lua::lua)
    # Every source of the library: lua.c and luac.c are the standalone interpreter and compiler
    file(GLOB lua_sources ${lua_SOURCE_DIR}/src/*.c)
    list(REMOVE_ITEM lua_sources ${lua_SOURCE_DIR}/src/lua.c ${lua_SOURCE_DIR}/src/luac.c)

    add_library(lua STATIC ${lua_sources})
    add_library(lua::lua ALIAS lua)

    target_include_directories(lua SYSTEM PUBLIC ${lua_SOURCE_DIR}/src)
    set_target_properties(lua PROPERTIES C_STANDARD 99 POSITION_INDEPENDENT_CODE ON)

    if(UNIX)
        # mkstemp instead of tmpnam, which the linker warns about
        target_compile_definitions(lua PRIVATE LUA_USE_POSIX)
        target_link_libraries(lua PUBLIC m)
    endif()
endif()

if(NOT TARGET sol2::sol2)
    add_library(sol2 INTERFACE)
    add_library(sol2::sol2 ALIAS sol2)

    target_include_directories(sol2 SYSTEM INTERFACE ${sol2_SOURCE_DIR}/include)
    # A Lua error or a wrong type becomes a result to check instead of undefined behaviour
    target_compile_definitions(sol2 INTERFACE SOL_ALL_SAFETIES_ON=1)
    target_link_libraries(sol2 INTERFACE lua::lua)
endif()
