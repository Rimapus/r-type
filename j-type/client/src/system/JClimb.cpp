/*
** EPITECH PROJECT, 2026
** j-type/client/src/system/JClimb
** File description:
** JClimb source file
*/

#include "system/JClimb.hpp"

#include "jtype/common/Log.hpp"
#include "jtype/common/Paths.hpp"

namespace jtype {

namespace {

#ifdef _WIN32
const std::filesystem::path JCLIMB_PROGRAM = "j-climb.exe";
#else
const std::filesystem::path JCLIMB_PROGRAM = "j-climb";
#endif

} // namespace

std::filesystem::path getJClimbFolder(void)
{
    return executableDir() / "j-climb";
}

void startJClimb(Process &jclimb)
{
    if (jclimb.isRunning()) {
        return;
    }
    const std::filesystem::path folder = getJClimbFolder();

    if (jclimb.start(folder / JCLIMB_PROGRAM, {(folder / "assets").string()})) {
        logger().info("J-Climb started");
    } else {
        logger().warn("Cannot start J-Climb from {}", (folder / JCLIMB_PROGRAM).string());
    }
}

} // namespace jtype
