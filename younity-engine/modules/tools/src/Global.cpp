/*
** EPITECH PROJECT, 2026
** younity-engine/modules/tools/src/Global
** File description:
** Global source file
*/

#include "younity/tools/Global.hpp"

namespace younity::tools {

MetricsStore &metrics(void)
{
    // Created at the first call: the C++ standard makes this initialization thread safe
    static MetricsStore globalMetrics;

    return globalMetrics;
}

Console &console(void)
{
    // Created at the first call: the C++ standard makes this initialization thread safe
    static Console globalConsole;

    return globalConsole;
}

} // namespace younity::tools
