/*
** EPITECH PROJECT, 2026
** j-type/client/src/scenes/ControlsScene
** File description:
** ControlsScene header file
*/

#pragma once

#include <cstddef>
#include <optional>

#include "input/Bindings.hpp"
#include "scenes/Context.hpp"
#include "scenes/MenuScreen.hpp"

namespace jtype {

/**
 * @brief The controls screen: each action of the game with its key and its gamepad input. Confirm on an action waits
 *        for the next key or gamepad input and binds it, a reserved key is skipped, back cancels. RESET gives back the default bindings, BACK goes
 *        back to the settings, which saves them.
 */
class ControlsScene : public MenuScreen
{
public:
    /**
     * @brief Create the controls screen, with the bindings of the settings of the context.
     *
     * @param context What the scenes share
     */
    explicit ControlsScene(Context &context);

private:
    bool interceptInput(void) override;
    void confirm(std::size_t line) override;
    void back(void) override;

    /**
     * @brief Stop waiting for an input, apply the bindings to the controls and show them.
     */
    void finishCapture(void);

    /**
     * @brief Write the binding of every action again, e.g. after two actions swapped a key.
     */
    void drawBindings(void);

    // The action waiting for its new key or gamepad input
    std::optional<GameAction> _capturing;
};

} // namespace jtype
