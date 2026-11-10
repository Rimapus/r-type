# Run the tests

The unit tests are written with [Criterion](https://github.com/Snaipe/Criterion). They run on Linux and macOS.

On Linux, Criterion is downloaded with the other dependencies. On macOS, install it first:

```sh
brew install criterion
```

## Build and run

```sh
cmake --preset tests
cmake --build --preset tests
```

The second command builds the tests and runs them. The last line gives the result, where `N` is the number of tests:

```text
[====] Synthesis: Tested: N | Passing: N | Failing: 0 | Crashing: 0
```

Every test passes when `Failing` and `Crashing` are 0.

## Run a part of the tests

The tests are one program, `build/tests/tests/younity_tests`. Each test belongs to a suite named after what it checks, like `ecs_registry` or `physics_collision_system`.

```sh
./build/tests/tests/younity_tests --filter='ecs_registry/*' --verbose
```

## Where the tests are

Each module has its folder in `tests/`, with one file per class: `tests/ecs/Registry.cpp` tests `younity::ecs::Registry`. The tests are short, and they show how every part of the engine is used.
