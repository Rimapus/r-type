/*
** EPITECH PROJECT, 2026
** younity-engine/tests/input/InputManager
** File description:
** InputManager unit tests
*/

#include <criterion/criterion.h>

#include <map>
#include <memory>
#include <set>
#include <stdexcept>
#include <vector>

#include "younity/input/InputManager.hpp"

using younity::input::AxisDirection;
using younity::input::GamepadAxis;
using younity::input::IInputSource;
using younity::input::InputManager;
using younity::input::Key;

namespace {

// What is held down on the fake keyboard and gamepad, changed by the tests
struct FakeDevices
{
    std::set<Key> keys;
    std::set<unsigned int> buttons;
    std::map<GamepadAxis, float> axes;
};

// Reads the fake devices instead of the real ones
class FakeInputSource : public IInputSource
{
public:
    explicit FakeInputSource(const FakeDevices &devices)
        : _devices(devices) {}

    void update(void) override {}

    [[nodiscard]] bool isKeyDown(Key key) const override
    {
        return this->_devices.keys.contains(key);
    }

    [[nodiscard]] bool isGamepadButtonDown(unsigned int button) const override
    {
        return this->_devices.buttons.contains(button);
    }

    [[nodiscard]] float getGamepadAxis(GamepadAxis axis) const override
    {
        const auto it = this->_devices.axes.find(axis);

        return it != this->_devices.axes.end() ? it->second : 0.0F;
    }

private:
    const FakeDevices &_devices;
};

} // namespace

Test(input_manager, null_source_throws)
{
    cr_assert_throw(InputManager(std::unique_ptr<IInputSource>{}), std::invalid_argument);
}

Test(input_manager, action_is_pressed_while_its_key_is_down)
{
    FakeDevices devices;
    InputManager input(std::make_unique<FakeInputSource>(devices));

    input.bindKey("shoot", Key::Space);
    devices.keys.insert(Key::Space);
    input.update();
    cr_assert(input.isActionPressed("shoot"));

    devices.keys.erase(Key::Space);
    input.update();
    cr_assert_not(input.isActionPressed("shoot"));
}

Test(input_manager, action_without_binding_is_not_pressed)
{
    FakeDevices devices;
    InputManager input(std::make_unique<FakeInputSource>(devices));

    devices.keys.insert(Key::Space);
    input.update();
    cr_assert_not(input.isActionPressed("shoot"));
}

Test(input_manager, any_bound_key_presses_the_action)
{
    FakeDevices devices;
    InputManager input(std::make_unique<FakeInputSource>(devices));

    input.bindKey("move_left", Key::Left);
    input.bindKey("move_left", Key::Q);
    devices.keys.insert(Key::Q);
    input.update();
    cr_assert(input.isActionPressed("move_left"));
}

Test(input_manager, just_pressed_only_at_the_first_update)
{
    FakeDevices devices;
    InputManager input(std::make_unique<FakeInputSource>(devices));

    input.bindKey("shoot", Key::Space);
    devices.keys.insert(Key::Space);
    input.update();
    cr_assert(input.isActionJustPressed("shoot"));

    input.update();
    cr_assert_not(input.isActionJustPressed("shoot"));
    cr_assert(input.isActionPressed("shoot"));
}

Test(input_manager, just_released_when_the_key_goes_up)
{
    FakeDevices devices;
    InputManager input(std::make_unique<FakeInputSource>(devices));

    input.bindKey("shoot", Key::Space);
    devices.keys.insert(Key::Space);
    input.update();
    cr_assert_not(input.isActionJustReleased("shoot"));

    devices.keys.erase(Key::Space);
    input.update();
    cr_assert(input.isActionJustReleased("shoot"));

    input.update();
    cr_assert_not(input.isActionJustReleased("shoot"));
}

Test(input_manager, gamepad_button_presses_the_action)
{
    FakeDevices devices;
    InputManager input(std::make_unique<FakeInputSource>(devices));

    input.bindGamepadButton("shoot", 0);
    devices.buttons.insert(0);
    input.update();
    cr_assert(input.isActionPressed("shoot"));
}

Test(input_manager, gamepad_axis_presses_the_action_past_halfway)
{
    FakeDevices devices;
    InputManager input(std::make_unique<FakeInputSource>(devices));

    input.bindGamepadAxis("move_right", GamepadAxis::X, AxisDirection::Positive);
    devices.axes[GamepadAxis::X] = 0.2F;
    input.update();
    cr_assert_not(input.isActionPressed("move_right"));

    devices.axes[GamepadAxis::X] = 0.8F;
    input.update();
    cr_assert(input.isActionPressed("move_right"));
}

Test(input_manager, gamepad_axis_only_presses_the_action_of_its_side)
{
    FakeDevices devices;
    InputManager input(std::make_unique<FakeInputSource>(devices));

    input.bindGamepadAxis("move_left", GamepadAxis::X, AxisDirection::Negative);
    input.bindGamepadAxis("move_right", GamepadAxis::X, AxisDirection::Positive);
    devices.axes[GamepadAxis::X] = -0.8F;
    input.update();
    cr_assert(input.isActionPressed("move_left"));
    cr_assert_not(input.isActionPressed("move_right"));
}

Test(input_manager, action_can_be_bound_to_another_key)
{
    FakeDevices devices;
    InputManager input(std::make_unique<FakeInputSource>(devices));

    input.bindKey("shoot", Key::Space);
    input.unbind("shoot");
    input.bindKey("shoot", Key::Enter);

    devices.keys.insert(Key::Space);
    input.update();
    cr_assert_not(input.isActionPressed("shoot"));

    devices.keys.insert(Key::Enter);
    input.update();
    cr_assert(input.isActionPressed("shoot"));
}

Test(input_manager, gives_what_is_bound_in_order)
{
    FakeDevices devices;
    InputManager input(std::make_unique<FakeInputSource>(devices));

    input.bindKey("jump", Key::Space);
    input.bindGamepadAxis("jump", GamepadAxis::Y, AxisDirection::Negative);
    input.bindKey("jump", Key::C);
    input.bindGamepadButton("jump", 0);

    const InputManager::Bindings bindings = input.getBindings("jump");

    cr_assert(bindings.keys == (std::vector{Key::Space, Key::C}));
    cr_assert(bindings.buttons == std::vector<unsigned int>{0});
    cr_assert_eq(bindings.axes.size(), 1U);
    cr_assert(bindings.axes[0].axis == GamepadAxis::Y);
    cr_assert(bindings.axes[0].direction == AxisDirection::Negative);
}

Test(input_manager, unbound_action_has_no_binding)
{
    FakeDevices devices;
    InputManager input(std::make_unique<FakeInputSource>(devices));

    input.bindKey("jump", Key::Space);
    input.unbind("jump");

    const InputManager::Bindings bindings = input.getBindings("jump");

    cr_assert(bindings.keys.empty());
    cr_assert(bindings.buttons.empty());
    cr_assert(bindings.axes.empty());
}
