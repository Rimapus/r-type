# Provides the criterion::criterion target used by the unit tests.
#   Linux x86_64: the official prebuilt release is fetched with CPM.
#   Other platforms: an installed Criterion is used if found (e.g. Homebrew on macOS).

if(CMAKE_SYSTEM_NAME STREQUAL "Linux" AND CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|AMD64")
    include(${CMAKE_CURRENT_LIST_DIR}/CPM.cmake)

    CPMAddPackage(
        NAME criterion
        URL https://github.com/Snaipe/Criterion/releases/download/v2.4.3/criterion-2.4.3-linux-x86_64.tar.xz
        URL_HASH SHA256=f1b3dd5186783dcdd63433c1facd3b4d6af5244a151057370b53bdda80f16121
        DOWNLOAD_ONLY YES
    )
    set(CRITERION_INCLUDE_DIR ${criterion_SOURCE_DIR}/include)
    set(CRITERION_LIBRARY ${criterion_SOURCE_DIR}/lib/libcriterion.so)
else()
    find_path(CRITERION_INCLUDE_DIR criterion/criterion.h)
    find_library(CRITERION_LIBRARY criterion)
endif()

if(NOT CRITERION_INCLUDE_DIR OR NOT CRITERION_LIBRARY)
    message(FATAL_ERROR "Criterion not found: install it or set JCLIMB_BUILD_TESTS to OFF")
endif()

add_library(criterion::criterion SHARED IMPORTED)
set_target_properties(criterion::criterion PROPERTIES
    IMPORTED_LOCATION ${CRITERION_LIBRARY}
    INTERFACE_INCLUDE_DIRECTORIES ${CRITERION_INCLUDE_DIR}
)
