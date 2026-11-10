# Packages the game with CPack: the binaries and the assets folder side by side, in one archive.
#   Linux: .tar.gz    Windows: .zip
# Only the "game" install component is packaged: the dependencies (e.g. SFML) come with install
# rules of their own, for headers and libraries the players do not need.

# Windows: the MSVC runtime DLLs next to the binaries, for the machines without Visual Studio
set(CMAKE_INSTALL_SYSTEM_RUNTIME_DESTINATION .)
set(CMAKE_INSTALL_SYSTEM_RUNTIME_COMPONENT game)
include(InstallRequiredSystemLibraries)

set(CPACK_PACKAGE_NAME j-type)
set(CPACK_INSTALL_CMAKE_PROJECTS "${PROJECT_BINARY_DIR};${PROJECT_NAME};game;/")

if(WIN32)
    set(CPACK_GENERATOR ZIP)
else()
    set(CPACK_GENERATOR TGZ)
endif()

include(CPack)
