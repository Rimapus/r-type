/*
** EPITECH PROJECT, 2026
** j-type/tests/client/Controls
** File description:
** Unit tests of the keys and the gamepad inputs read by the client
*/

#include <criterion/criterion.h>

#include <memory>
#include <set>

#include "input/Controls.hpp"
#include "jtype/common/Input.hpp"
#include "younity/input/GamepadAxis.hpp"
#include "younity/input/IInputSource.hpp"
#include "younity/input/Key.hpp"

using jtype::Controls;
using jtype::MenuAction;
using younity::input::GamepadAxis;
using younity::input::IInputSource;
using younity::input::Key;

namespace {

// A keyboard whose keys held down are chosen by the test, and no gamepad
class FakeKeyboard : public IInputSource
{
public:
    explicit FakeKeyboard(const std::set<Key> &keys)
        : _keys(keys) {}

    void update(void) override {}

    [[nodiscard]] bool isKeyDown(Key key) const override
    {
        return this->_keys.contains(key);
    }

    [[nodiscard]] bool isGamepadButtonDown(unsigned int /* button */) const override
    {
        return false;
    }

    [[nodiscard]] float getGamepadAxis(GamepadAxis /* axis */) const override
    {
        return 0.0F;
    }

private:
    const std::set<Key> &_keys;
};

} // namespace

Test(client_controls, reads_the_actions_held_down)
{
    const std::set<Key> keys{Key::Space, Key::Enter};
    Controls controls(std::make_unique<FakeKeyboard>(keys));

    controls.update();
    cr_assert_eq(controls.read(), jtype::input::SHOOT);
    cr_assert(controls.isMenuPressed(MenuAction::Confirm));
    cr_assert(controls.getKeyJustPressed().has_value());
}

Test(client_controls, disabled_reads_nothing)
{
    const std::set<Key> keys{Key::Space, Key::Enter, Key::M};
    Controls controls(std::make_unique<FakeKeyboard>(keys));

    controls.setEnabled(false);
    controls.update();
    cr_assert_eq(controls.read(), 0);
    cr_assert_not(controls.isMenuPressed(MenuAction::Confirm));
    cr_assert_not(controls.isMutePressed());
    cr_assert_not(controls.getKeyJustPressed().has_value());
    cr_assert_not(controls.getGamepadJustPressed().has_value());
}

Test(client_controls, enabled_again_reads_the_keys_held_down)
{
    const std::set<Key> keys{Key::Space};
    Controls controls(std::make_unique<FakeKeyboard>(keys));

    controls.setEnabled(false);
    controls.update();
    controls.setEnabled(true);
    controls.update();
    cr_assert_eq(controls.read(), jtype::input::SHOOT);
}
