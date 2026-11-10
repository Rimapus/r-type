# Input

The `input` module answers one question for the game: is this action pressed? The game talks about actions, like `shoot` or `move_left`, and the module knows which keys, buttons and sticks trigger them.

The headers are in `modules/input/include/younity/input/`, in the namespace `younity::input`.

## Actions and bindings

An action is a name. A binding links it to a key of the keyboard, a button of the gamepad, or one side of a gamepad axis. An action can have as many bindings as needed.

```cpp
input::InputManager inputs;

inputs.bindKey("shoot", input::Key::Space);
inputs.bindGamepadButton("shoot", 0);
inputs.bindKey("move_left", input::Key::Left);
inputs.bindGamepadAxis("move_left", input::GamepadAxis::X, input::AxisDirection::Negative);
```

Because the game only knows the names, the bindings can change while it runs. A settings screen removes the old ones and adds the new one:

```cpp
inputs.unbind("shoot");
inputs.bindKey("shoot", input::Key::Enter);
```

An axis counts as pressed once the stick is pushed past the middle of its range on the bound side.

`getBindings(name)` gives back what is bound to an action: its keys, its buttons and its axes, in the order they were bound. A controls screen writes them with `describe`, from `Names.hpp`: the name of each key, then `Button 0` for a button and `X-` or `X+` for a side of an axis.

```cpp
const std::vector<std::string> names = input::describe(inputs.getBindings("shoot")); // {"Space", "Button 0"}
```

`Names.hpp` also names each `Key` and `GamepadAxis` as written in its enum, e.g. `Space`, `LShift`, `Num1` or `PovX`, and finds them back by that name: `parseKey("Space")` and `parseGamepadAxis("X")` return the value, or nothing for an unknown name. The [scripts](scripting.md#actions-and-logs) bind the actions with these names.

## Reading the actions

`update()` reads the devices once per frame. The three questions below then give the same answer during the whole frame.

| Question                       | True when                                              |
|--------------------------------|--------------------------------------------------------|
| `isActionPressed(name)`        | The action is held down                                |
| `isActionJustPressed(name)`    | The action went down at this frame                     |
| `isActionJustReleased(name)`   | The action went up at this frame                       |

`isActionPressed` suits a movement, `isActionJustPressed` suits a shot fired once per press.

```cpp
inputs.update();
if (inputs.isActionPressed("move_left")) {
    velocity.value.x = -200.0F;
}
if (inputs.isActionJustPressed("shoot")) {
    // create a missile
}
```

## Where the state comes from

`InputManager` reads the devices through an interface, `IInputSource`, with three questions: is this key down, is this button down, where is this axis.

`SfmlInputSource` answers them with the real keyboard and gamepads. It is the default source and the only file of the module that uses the graphics library.

The unit tests give the manager a fake source that they control, which is how "just pressed" and rebinding are tested automatically. The same door is open for a source that replays recorded inputs.

## Main types

| Type              | Role                                                       |
|-------------------|------------------------------------------------------------|
| `InputManager`    | Bindings and state of the actions                          |
| `Names.hpp`       | The names of the keys and axes, and `describe` for a controls screen |
| `Key`             | The keys of the keyboard that can be bound                 |
| `GamepadAxis`     | The axes of a gamepad: `X` and `Y` are the left stick      |
| `AxisDirection`   | The side of an axis that triggers an action                |
| `IInputSource`    | Where the state of the devices is read                     |
| `SfmlInputSource` | The real keyboard and gamepads                             |
