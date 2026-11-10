# Core

`core` holds the small types that every other module needs: math, colors, time, events and the main loop. It depends on the standard library only.

All its types live in the namespace `younity`, and its headers are in `modules/core/include/younity/core/`.

## Math types

`Vector2<T>` is a point or a direction, `Rect<T>` is a rectangle with a position and a size. Both are plain structs with public members, and both accept number types only, through the `Numeric` concept.

| Type      | Members                          | Shortcuts              |
| --------- | -------------------------------- | ---------------------- |
| `Vector2` | `x`, `y`                         | `Vector2f`, `Vector2i` |
| `Rect`    | `left`, `top`, `width`, `height` | `Rectf`, `Recti`       |

```cpp
younity::Vector2f position{10.0F, 20.0F};
const younity::Vector2f velocity{100.0F, 0.0F};
const younity::Rectf screen{0.0F, 0.0F, 800.0F, 600.0F};

position += velocity * 0.5F;        // (60, 20)
screen.contains(position);          // true
```

`Vector2` has the usual operators, `length()` and `normalized()`. `Rect` has `contains(point)` and `intersects(other)`, which is the test behind the collision detection.

`Color` holds `r`, `g`, `b` and `a`, each from 0 to 255, white and opaque by default. It lives in `core` so the [component registry](ecs.md#component-registry) can read and write it.

These types are the engine's own. The graphics library has similar ones, and the `render` module converts between the two, so the rest of the engine and the game stay independent from it.

## Version

`version()` gives the version of the engine as a `std::string_view`, e.g. `"{{ younity_version }}"`:

```cpp
std::cout << "Younity " << younity::version() << '\n';
```

CMake sets it from the `VERSION` of the `project()` in the root `CMakeLists.txt`, which is the single source of truth: the build defines `YOUNITY_VERSION` from it, and the [debug window](render.md#debug-interface) shows the same string.

## Time

`Clock` measures elapsed time on the monotonic clock of the system. Restarted once per frame, it gives the delta time:

```cpp
younity::Clock clock;

const float dt = clock.restart();   // seconds since the last restart
```

`FixedTimestep` turns this variable time into steps of a fixed duration. The game logic runs once per step, so it behaves the same at any frame rate.

```cpp
younity::FixedTimestep timestep(1.0F / 60.0F);

for (std::size_t steps = timestep.advance(dt); steps > 0; --steps) {
    registry.updateSystems(timestep.getStep());
}
```

A very long frame, after a breakpoint for example, is shortened to a quarter of a second by default. The game then slows down for a moment instead of running dozens of steps to catch up.

`Application` runs both for you, see [Main loop](#main-loop).


## Events

`EventBus` lets two parts of the program talk while staying independent. One side publishes an event, the other side subscribed a function to that type of event. An event is any struct.

```cpp
struct ScoreChanged
{
    int score;
};

younity::EventBus bus;

bus.subscribe<ScoreChanged>([](const ScoreChanged &event) {
    std::cout << "score: " << event.score << '\n';
});
bus.publish(ScoreChanged{100});     // prints "score: 100"
```

`publish` calls every subscriber of the type at once, in the order they subscribed, before it returns.

The engine uses it to announce what happens: the collision system publishes a `CollisionEvent`, and the game decides what a collision means. The module that detects and the code that reacts stay unaware of each other.

Each scene owns a bus, reached with `Scene::getEventBus()`. Its subscriptions end with the scene.

## Main loop

`Application` is the loop of the game. The game derives from it, fills its hooks, and calls `run()`:

```cpp
class MyGame : public younity::Application
{
public:
    MyGame(void) : younity::Application(1.0F / 60.0F) {}

protected:
    void onStart(void) override { scenes.changeScene<MenuScene>(); }
    void onInput(void) override;               // window events, inputs, packets
    void onFixedUpdate(float step) override { scenes.update(step); }
    void onRender(float alpha) override;       // drawing
    void onStop(void) override;                // saving, closing the connection
};

MyGame game;

game.run();
```

`run()` calls `onStart()` once, then repeats the frame below until `stop()` is called, then calls `onStop()` once.

| Hook            | Called                                | Typical use                         |
| --------------- | ------------------------------------- | ----------------------------------- |
| `onInput`       | First in each frame                   | Window events, inputs, packets      |
| `onFixedUpdate` | Once per fixed step, possibly 0 times | `scenes.update(step)`               |
| `onRender`      | Last in each frame                    | Drawing, with `alpha`               |

Only `onFixedUpdate` is required, the other hooks do nothing by default. A dedicated server has no window: it reads its packets in `onInput` and leaves `onRender` empty.

`stop()` ends the loop once the current frame is over. Called from `onInput`, for example when the window closes, it skips the rest of the frame, so nothing is drawn into a closed window. It can be called from another thread, to stop a server on Ctrl+C.

`onStop()` is called even when a hook throws, `onStart()` included, and the exception then goes on to the caller of `run()`. A `stop()` that arrives while `run()` starts is kept: the loop ends before its first frame. The time spent in `onStart()`, loading the first scene for example, is not simulated.

The loop does not sleep. On a client, the vertical sync of the window paces it.
