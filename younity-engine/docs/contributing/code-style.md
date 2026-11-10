# Code style

Two tools hold the style of the code, and the pipeline runs both: `clang-format` for the layout, `clang-tidy` for the naming and a few checks.

## Format

Check the format of the whole engine:

```sh
find modules -type f \( -name '*.cpp' -o -name '*.hpp' \) -print0 | xargs -0 clang-format --dry-run --Werror
```

Fix a file in place:

```sh
clang-format -i modules/ecs/src/Registry.cpp
```

The rules are in `.clang-format`. The main ones: four spaces, the opening brace of a function, a class or a struct on its own line, and the opening brace of an `if`, a loop, a namespace or an enum at the end of the line.

## Lint

```sh
cmake --preset default
find modules -type f -name '*.cpp' -print0 | xargs -0 clang-tidy -p build/default --quiet --header-filter='.*/modules/.*'
```

The rules are in `.clang-tidy`.

## Naming

| What                              | Form            | Example             |
|-----------------------------------|-----------------|---------------------|
| Class, struct, enum               | `PascalCase`    | `SceneManager`      |
| Function and variable             | `camelCase`     | `createEntity`      |
| Private or protected member       | `_camelCase`    | `_activeScene`      |
| Public member of a struct         | `camelCase`     | `position`          |
| Constant                          | `UPPER_CASE`    | `MAX_COMPONENTS`    |
| Namespace                         | `younity::<module>` | `younity::ecs`  |

## Conventions

- A member is always reached through `this->`.
- A function with zero parameter is written `name(void)`.
- Every statement of an `if` or a loop is in braces.
- A getter starts with `get`, a test with `is` or `has`.
- A type whose members can take any value is a `struct` with public members, like `Vector2`. A type that protects a rule is a `class` with private members, like `Registry`.
- A class works with another one through its public interface: `friend` declarations are left out of the code.
- A component is registered in the `registerComponents` function of its module, with every field of a `FieldValue` type, and `tests/<module>/Components.cpp` lists them. See the [component registry](../architecture/ecs.md#component-registry).

## Comments

Every public class and function has a Doxygen comment, which feeds the [API reference](../api/annotated.md):

```cpp
/**
 * @brief Check if the entity has been created and not destroyed yet.
 *
 * @param entity The entity to check
 *
 * @return true if the entity is alive.
 */
[[nodiscard]] bool isAlive(Entity entity) const noexcept;
```

A comment inside a function explains why the code is written this way, when the reason is hard to guess from the code.
