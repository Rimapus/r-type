# Tools

The `tools` module holds the developer tools of the engine that do not draw anything. Today it provides the metrics store, and the console variables and commands. It depends on `core` only, so a dedicated server can publish its metrics and run its commands without a window.

The headers are in `modules/tools/include/younity/tools/`, in the namespace `younity::tools`.

## Metrics

A metric is a named series of samples. The store keeps the last 120 samples of each metric, two seconds at 60 frames per second, unless it is created with another history size.

```cpp
younity::tools::MetricsStore metrics;

metrics.record("frame_time", dt);          // a value, in seconds
metrics.increment("packets_lost");         // a counter, from 0
{
    younity::tools::ScopedTimer timer(metrics, "physics");
    scene.update(step);                    // the duration is recorded when the timer ends
}
```

| Type           | Role                                                            |
|----------------|-----------------------------------------------------------------|
| `MetricsStore` | Owns the metrics, records the samples and keeps their history    |
| `Metric`       | A copy of one metric: name, kind, history and statistics         |
| `MetricKind`   | `Value`, `Counter` or `Timer`, to format the metric              |
| `ScopedTimer`  | Records the duration of a scope as a `Timer` metric              |
| `metrics()`    | The store shared by the engine and the game                      |

`record` adds a sample to a value or a timer. `increment` adds to a counter: the history keeps the total after each call, so `last` is the current count. The kind of a metric is set by its first sample and does not change.

`getMetric` returns a `Metric` or `std::nullopt` when the name is unknown, and `getMetrics` returns every metric, sorted by name. Both return a copy, so a reader can display the metrics while another thread publishes new samples.

The store is thread safe: a module can publish from its own thread, for example the network pump, while the game reads the history. `MetricsStore(300)` keeps 300 samples instead of 120. `clear()` removes every metric.

`ScopedTimer` measures in seconds, like `younity::Clock`. It records when the scope ends; `stop()` records earlier and the destructor does nothing after that.

The engine and the game reach the shared store with `younity::tools::metrics()`:

```cpp
younity::tools::metrics().increment("network.packets_lost");
```

The store has no dependency on the window or on Dear ImGui: a server publishes and reads its metrics the same way as a client.

## Metrics of the engine

The engine publishes these metrics by itself:

| Metric                 | Kind      | What it measures                                              |
|------------------------|-----------|---------------------------------------------------------------|
| `fps`                  | `Value`   | Frames per second of the render window                        |
| `frame_time`           | `Timer`   | Time between two frames of the render window, in seconds      |
| `system.<name>`        | `Timer`   | Time a system took, when the game sets a profiler on its registry |
| `network.latency`      | `Timer`   | Round trip of a ping, in seconds                              |
| `network.packets_lost` | `Counter` | Missing sequence numbers of the server, cumulative            |

`render::Window::clear()` records `frame_time` and `fps` once per frame. The game records `system.<name>` by setting a profiler on its registry, see [profiling the systems](ecs.md#profiling-the-systems). `network::Session` records `network.latency` for each answered ping and adds the holes of the server sequence to `network.packets_lost`; it writes to the shared store by default and its other constructor takes another store. The [editor](editor.md#metrics-overlay) draws them in its metrics overlay.

## Console

A console holds named variables and commands. A module or the game adds them once, then reads, changes and runs them while the game runs. The console draws nothing and depends on no UI, so a server uses it like a client. The [editor](editor.md#developer-console) module draws a window over it.

```cpp
younity::tools::console().addVariable<bool>("render.wireframe", false, "Draw the edges of the sprites");
younity::tools::console().addVariable<float>("audio.volume", 0.5F, "Sound level");

younity::tools::console().addCommand(
    "hello",
    "Print a greeting",
    [](std::span<const std::string_view>) -> std::expected<std::string, younity::tools::ConsoleError> {
        return std::string("hello");
    });
```

A variable is a `bool`, an `int`, a `float` or a `std::string`. A name is case sensitive and cannot contain a space. Variables and commands share one namespace, so a name is used once, and `help` is reserved. The convention is `module.setting`, like the metric names. Adding a duplicate name, an empty name, `help`, an empty command handler or a non-finite float default throws `std::invalid_argument`.

`getVariable` returns a copy or `std::nullopt`, and `getVariables` returns every variable sorted by name. `getValue<T>` returns the typed value and throws `std::out_of_range` for an unknown name or `std::bad_variant_access` for another type. `setValue` throws `std::invalid_argument` when the value has another type or is a non-finite float. `resetVariable` and `resetVariables` put the defaults back, and `clear` removes everything.

`getCommands` returns the names and descriptions sorted by name.

`execute` treats its argument as text from outside the code, so it returns a `std::expected` instead of throwing:

```cpp
const std::expected<std::string, younity::tools::ConsoleError> output =
    younity::tools::console().execute("render.wireframe true");
// output.value() == "render.wireframe = true"
```

A command runs its handler with the arguments after its name. A bare variable name prints it as `name = value`, and a variable name followed by one value sets it. A string value is printed in double quotes, with `\"` and `\\` escaped, so the printed line can be run again. The arguments are separated by spaces. A double-quoted argument keeps its spaces and accepts `\"` and `\\`; an unclosed quote or another escape is an error. Values are strict: a `bool` is `true`, `false`, `1` or `0`; an `int` is base 10 and must fit; a `float` is finite. Extra arguments, unknown names and bad values come back as a `ConsoleError` and leave the variables unchanged.

`help` is the command of every console (`HELP_COMMAND`): it lists the commands, then the variables with their values, each with its description, so a player finds what the console can do without reading the code.

```text
Commands:
  hello - Print a greeting
Variables:
  audio.volume = 0.5 - Sound level
  render.wireframe = false - Draw the edges of the sprites
```

`registerCommands` adds the commands of the engine, so every game has a console to start from: `metrics` prints the last value and the average of every metric, or of the one named after it, and `log.level <logger> <level>` changes the level of a logger at run time, e.g. `log.level network DEBUG`. `removeCommand` takes a command back, e.g. when the object it acts on is destroyed.

```cpp
younity::tools::registerCommands(younity::tools::console(), younity::tools::metrics(), younity::logger::registry());
```

A command returns the text to show, or a `ConsoleError`:

```cpp
console.addCommand("fail", "Always fails", [](std::span<const std::string_view>)
    -> std::expected<std::string, younity::tools::ConsoleError> {
    return std::unexpected(younity::tools::ConsoleError{"it failed"});
});
```

`Console` has no lock and no change callback: use it from the thread that runs the game loop, and read a variable when it is needed. The engine and the game reach the shared console with `younity::tools::console()`.

| Type             | Role                                                               |
|------------------|--------------------------------------------------------------------|
| `Console`        | Owns the variables and commands, and runs a line                   |
| `Variable`       | A copy of one variable: name, description, type, value and default |
| `VariableType`   | `Bool`, `Int`, `Float` or `String`                                 |
| `VariableValue`  | A `std::variant` of `bool`, `int`, `float` and `std::string`       |
| `CommandInfo`    | The name and description of a command                              |
| `CommandHandler` | The function that runs a command, with its arguments               |
| `ConsoleError`   | Why a line could not be run                                        |
| `HELP_COMMAND`   | `help`, the reserved command listing the commands and variables    |
| `console()`      | The console shared by the engine and the game                      |
