/*
** EPITECH PROJECT, 2026
** j-type/client/src/scenes/Context
** File description:
** Context source file
*/

#include "scenes/Context.hpp"

#include "jtype/common/Log.hpp"
#include "settings/Colorblind.hpp"

namespace jtype {

void applyColorblind(Context &context)
{
    if (!context.window.setColorMatrix(getColorblindMatrix(context.settings.colorblind))) {
        logger().warn("The graphics card has no shaders, the colors cannot be corrected");
    }
}

} // namespace jtype
