# Editor

The `editor` module shows the world of the game while it runs, edits it live, lists its assets, and draws the developer console and the metrics overlay. It draws its windows in the [debug interface](render.md#debug-interface) of the render window, over the game, and fixes them in place in the [debug and editor layouts](#layouts).

The headers are in `modules/editor/include/younity/editor/`, in the namespace `younity::editor`. The module is built on `core`, `ecs`, `assets`, `render`, `input` and `tools`, and uses Dear ImGui through ImGui-SFML.

## Drawing the editor

An `Editor` receives the [component registry](ecs.md#component-registry) of the game, and draws the registry it is given once per frame, between `clear()` and `display()`:

```cpp
ecs::ComponentRegistry components;
editor::Editor editor(components);
editor::ConsoleWindow console(tools::console(), logger::registry());
editor::MetricsWindow metrics;

void onRender(float /* alpha */) override
{
    window.clear();
    if (scenes.hasActiveScene()) {
        render::drawEntities(scenes.getActiveScene().getRegistry(), window);
        editor.draw(scenes.getActiveScene().getRegistry(), window);
    }
    console.draw();
    metrics.draw();
    window.display();
}
```

| Window      | Shows                                                                                            |
|-------------|--------------------------------------------------------------------------------------------------|
| `Entities`  | The Play, Pause and Step buttons, then one line per alive entity, `Entity 3`; a click selects it |
| `Inspector` | The registered components of the selected entity, one folding section each, with their fields    |
| `Scene`     | The path of a scene file, Save and Load buttons, and the result of the last action               |
| `Console`   | The log messages, the command line, its history and the completion hints                         |
| `Metrics`   | Every metric of the store, with its last sample, its minimum, its average and its maximum        |

The selected entity is read with `getSelected()` and set with `select()`. An entity destroyed while selected is unselected at the next frame.

## Layouts

A `Workspace` lays out the windows of the editor, so they never have to be moved or resized by hand. The game chooses the key of each layout:

```cpp
editor::Workspace workspace(editor, console, metrics, browser);

workspace.bindKey(editor::Layout::Debug, input::Key::F1);
workspace.bindKey(editor::Layout::Editor, input::Key::F2);

void onRender(float /* alpha */) override
{
    window.clear();
    render::drawEntities(scenes.getActiveScene().getRegistry(), window);
    workspace.draw(scenes.getActiveScene().getRegistry(), window);
    window.display();
}
```

| Layout   | Shows                                                                                                   | The game reads its inputs                                 |
|----------|---------------------------------------------------------------------------------------------------------|-----------------------------------------------------------|
| `Game`   | The game in the whole window, without the debug interface                                               | Always                                                    |
| `Debug`  | The game in the whole window, `Younity` at the top left, `Metrics` at the top right, `Console` along the bottom | Unless a field is typed in, like the command line of the console |
| `Editor` | The preview of the game at the top center, `Entities` over `Scene` on its left, `Inspector` on its right, `Console`, `Assets` and `Metrics` along the bottom | From a click in the preview to a click on a window |

The workspace starts with the game layout. The key of a layout shows it, pressed again it goes back to the game layout, and the key of the other layout switches straight to the other one: the debug and the editor layouts are never shown together. `toggle(layout)` does the same from the code, and `getLayout()` tells the layout shown. The keys are read from the events of the window, so they work while the window has the focus.

`getDebugAreas` and `getEditorAreas` place the windows from the size of the window, again at each frame, so a resized window keeps them in place; the mouse cannot move, resize or fold them. In the editor layout, the game is shown in the preview with [`setGameArea`](render.md#game-area): the selection, the drag and the dropped assets land on the entity under the cursor. The debug layout selects and drags nothing.

The game asks the workspace before it reads its inputs:

```cpp
if (workspace.hasGameInput()) {
    // move the player
}
```

`Editor::draw`, `ConsoleWindow::draw`, `MetricsWindow::draw` and `AssetBrowser::draw` also take their areas themselves, for a game that places the windows another way.

## Selecting and moving entities

The mouse picks the entities in the game view, outside the editor windows. The window given to `draw` converts the cursor to the world with `mapPixelToWorld`, so the clicks stay on target in a resized window.

| Mouse                      | Effect                                                                            |
|----------------------------|-----------------------------------------------------------------------------------|
| Click on an entity         | Selects the entity on top at this point, found with [`getEntityAt`](render.md#entity-at-a-point) |
| Click on an empty place    | Clears the selection                                                              |
| Button held after a click  | Drags the entity: its `Transform` keeps its distance to the cursor                |
| Click on an editor window  | Belongs to the window: the selection stays                                        |

The selected entity is outlined in yellow, around its sprite and its text, under the editor windows. Dragging works while playing and while paused.

## Play, pause and step

The game asks the editor before each fixed step, and moves its scenes forward only when the editor lets it:

```cpp
void onFixedUpdate(float step) override
{
    if (editor.shouldUpdate()) {
        scenes.update(step);
    }
}
```

| Button | Acts          | Effect on `shouldUpdate()`                    |
|--------|---------------|-----------------------------------------------|
| Pause  | While playing | `false` at every step: the world is frozen    |
| Step   | While paused  | `true` for one fixed step, then `false` again |
| Play   | While paused  | `true` at every step                          |

The editor starts playing. While paused, the frames go on: the world is drawn and the inspector edits it. The steps held back during the pause are dropped, so the game resumes where it stopped. The same states are reached from the code with `pause()`, `step()`, `play()` and `isPaused()`.

## Developer console

A `ConsoleWindow` shows the variables and commands of a [console](tools.md#console), and the log messages written after it was created. It registers its own logger sink, so messages written from another thread are added safely; a registry has one console window. The sink is removed when the window is destroyed.

The window keeps the last 1000 lines of logs and command results by default. Pass another limit to the constructor when a game needs a longer or shorter scrollback:

```cpp
editor::ConsoleWindow console(tools::console(), logger::registry(), 250);
```

The input line runs commands and changes variables with `Console::execute`. A command line and its result or error stay in the scrollback. `Enter` runs the line, `Up` and `Down` recall the commands already run, and `Tab` completes the first word with a variable or command name, `help` included. When several names match, the window completes their common prefix and shows their names and descriptions below the input line.

The console itself has no lock, so create its variables and commands, and draw the window, from the game loop. Only the log sink is written from other threads.

## Metrics overlay

A `MetricsWindow` shows the [metrics](tools.md#metrics) of a store, sorted by name, with the last sample, the minimum, the average and the maximum of the kept history. A `Timer` is shown in milliseconds, a `Counter` as a whole number, and a `Value` with two decimals.

```cpp
editor::MetricsWindow metrics;   // reads tools::metrics()

void onRender(float /* alpha */) override
{
    window.clear();
    // draw the game and the other windows
    metrics.draw();
    window.display();
}
```

The default constructor reads the store shared by the engine and the game, `tools::metrics()`. The other constructor takes another store, which must outlive the window. The window reads the store once per frame, so it can show metrics published by the engine, the game or another thread.

The [render window](render.md#the-window) publishes `frame_time` and `fps` at each frame, and the [registry profiler](ecs.md#profiling-the-systems) publishes `system.<name>` when the game sets it.

## Asset browser

An `AssetBrowser` lists the textures and the sounds of the folder of the [resource manager](assets.md#the-resource-manager), its subfolders included, in a window of its own:

```cpp
assets::ResourceManager resources("assets");

resources.setLoader<render::Texture>(render::Texture::loadFromFile);

editor::AssetBrowser browser(resources);

// each frame, between window.clear() and window.display()
browser.draw(scenes.getActiveScene().getRegistry(), window);
```

A file is a texture or a sound by its extension, whatever its case: `.png`, `.jpg`, `.jpeg`, `.bmp`, `.tga` and `.gif` for the textures, `.wav`, `.ogg`, `.flac` and `.mp3` for the sounds. The other files are not listed. The browser lists the folder when it is created, and again with its Refresh button or `refresh()`.

A texture dragged from the list and dropped on the game, outside the windows of the debug interface, creates an entity in the registry: a `Transform` at the mouse and a `Sprite` of the texture, its origin at the center of the image. The texture goes through the resource manager, which needs a loader for `render::Texture`; a texture that cannot be loaded creates nothing. The position is the mouse converted to the world of the entities with `Window::mapPixelToWorld`, like the selection with the mouse, so a resized window places the entity under the cursor. A sound is listed but not dropped: no component of the engine holds a sound.

`drop(registry, asset, position)` creates the entity of an asset without the mouse, the way the window does.

## Scene files

The `Scene` window saves the registry being edited and opens a [scene file](ecs.md#scene-files). The path starts at `level.json` and can be changed in the field; Save and Load are disabled while it is empty. Only the component types registered in the component registry are written.

The functions behind the buttons are public, so a game can save or load without the window:

```cpp
const std::expected<void, ecs::SceneFileError> saved = editor.saveScene(registry, "level.json");
const std::expected<void, ecs::SceneFileError> loaded = editor.loadScene(registry, "level.json");
```

Save reports the file and the reason when it fails. Load is strict and checks the file before it changes the registry, so an invalid file leaves the scene and its systems untouched. A successful load clears the selection and replaces the entities and components with the ones of the file, under the ids they were saved with. The registry keeps its systems, so the game goes on with the loaded scene and the entities it holds name the same ones after a Save and a Load.

The Scene window shows the result of the last Save or Load, with the file, the line when there is one, and the reason. The window loads from the draw call, outside `updateSystems`.

A game whose world is not its own, like a networked client drawing what its server sends, hides the window: a loaded file could not stay in line with the entities the game holds or the next state the server sends. The entity list then takes its place in the editor layout.

```cpp
editor.setSceneWindowShown(false);
```

`registerCommands` adds `pause`, `play` and `step` to a console, the buttons of the editor as commands; the editor removes them when it is destroyed, and it cannot be copied, so no command acts on a copy.

```cpp
editor.registerCommands(younity::tools::console());
```

## Editing a field

Each field gets the widget of its [field type](ecs.md#component-registry), and a change is written to the component at once with `ComponentInfo::setField`.

| Field type             | Widget                                              |
|------------------------|-----------------------------------------------------|
| `bool`                 | Checkbox                                            |
| `int`                  | Number, dragged or typed                            |
| `float`                | Number, dragged or typed                            |
| `std::string`          | Text field, applied with `Enter`                    |
| `Vector2f`             | Two numbers                                         |
| `Color`                | Color picker, with the opacity                      |
| `std::optional<Recti>` | Checkbox to set it, then four numbers               |
| `std::vector<Recti>`   | Each rectangle with four numbers, Add frame and Remove |

An integer dragged past the range of its field, like 256 for a `std::uint8_t`, keeps the last valid value, and so does a value the field refuses, like the path of a texture that cannot be loaded. A text is applied when `Enter` is pressed, not at each letter: the path of a texture names a file only once it is typed whole.

## Main types

| Type                 | Role                                                                                              |
|----------------------|---------------------------------------------------------------------------------------------------|
| `Editor`             | Draws the entity list, inspector and Scene window, keeps the selection, pauses and steps the game |
| `Workspace`          | Switches between the layouts with keys, places their windows and shows the game in its area       |
| `Layout`             | The game alone, the debug layout or the editor layout                                             |
| `DebugAreas`, `EditorAreas` | Where `getDebugAreas` and `getEditorAreas` place the windows for a size of window           |
| `ConsoleWindow`      | Draws the console, captures the log messages and keeps the command input                          |
| `MetricsWindow`      | Draws the metrics of a store, with their current value and their statistics                       |
| `AssetBrowser`       | Lists the textures and the sounds of the asset folder, and drops a texture on the game            |
| `Asset`, `AssetKind` | A file listed by the asset browser, and whether it is a texture or a sound                        |
| `ASSET_BROWSER_TITLE` | Title and id of the asset browser window                                                          |
| `ENTITY_LIST_TITLE`  | Title and id of the entity list window                                                            |
| `INSPECTOR_TITLE`    | Title and id of the inspector window                                                              |
| `SCENE_TITLE`        | Title and id of the Scene window                                                                  |
| `CONSOLE_TITLE`      | Title and id of the console window                                                                |
| `METRICS_TITLE`      | Title and id of the metrics window                                                                |
