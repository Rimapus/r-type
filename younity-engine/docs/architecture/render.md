# Render

The `render` module puts the game on screen: the window, the textures, the sprites and the texts of the entities drawn layer by layer, the animations of the sprites, the particles, and the debug interface drawn over the game.

The headers are in `modules/render/include/younity/render/`, in the namespace `younity::render`. The module is built on `core`, `ecs`, `assets` and `tools`.

## The window

`Window` opens the window of the game and drives one frame of display.

```cpp
younity::render::Window window(800, 600, "My game");

while (window.isOpen()) {
    window.pollEvents();
    window.clear();
    // draw the frame
    window.display();
}
```

| Function       | What it does                                                    |
|----------------|-----------------------------------------------------------------|
| `isOpen()`     | Tells if the window is still open                               |
| `pollEvents()` | Handles the pending window events, like a click on the close button |
| `clear()`      | Erases the previous frame and publishes the frame metrics        |
| `display()`    | Shows what was drawn since the last `clear()`                   |
| `mapPixelToWorld(pixel)` | Converts a point of the window, like the mouse cursor, to the world of the entities |
| `mapWorldToPixel(world)` | Converts a point of the world to the window                      |
| `setLogicalResolution(width, height)` | Draws the game at a small fixed resolution, scaled to the window with sharp pixels |
| `setColorMatrix(matrix)` | Changes the colors of the game with a 3x3 matrix, see [Color matrix](#color-matrix) |
| `setGameArea(area)` | Shows the game in a part of the window, see [Game area](#game-area) |

`clear()` records two metrics in the [metrics store](tools.md#metrics), once per frame: `frame_time`, a Timer in seconds, and `fps`, a Value. The time is measured between two frames; a second `clear()` before `display()` keeps the same frame and records nothing.

The world keeps the size the window opened with. A resized window stretches it, so a pixel of the window and a point of the world differ: the two conversions follow the stretch.

### A logical resolution

A pixel-art game is drawn at a small fixed resolution, like 128x128, whatever the size of the window:

```cpp
younity::render::Window window(800, 600, "My game");

window.setLogicalResolution(128, 128);
```

The sprites and the texts are then drawn in an image of 128x128, and the positions of the entities are in its pixels: `(127, 127)` is the bottom-right corner. `display()` shows the image in the window, scaled by the largest whole factor that fits, so each pixel of the game is a square of screen pixels, and centered between black bars. In a window of 800x600, the factor is 4: the game takes 512x512 pixels, from `(144, 44)`. `getPixelPerfectArea()` computes this area; a window smaller than the logical resolution shrinks the game instead, and some rows and columns of its pixels are not shown. A minimized window shows nothing.

The debug interface is drawn over the window at its own size, not in the image. `mapPixelToWorld` and `mapWorldToPixel` convert between the window and the logical image, so the mouse cursor still lands on the entity under it; a point on a black bar is outside of the image.

### Color matrix

`setColorMatrix` changes the colors of everything the game draws, for example to correct them for a color blind player. The `ColorMatrix` has 3 rows of 3 numbers: each row makes a new channel, red, then green, then blue, from the red, the green and the blue of the pixel, each from 0 to 1. The alpha is kept, and the result is clamped between 0 and 1.

```cpp
// Swaps the red and the blue of every pixel
window.setColorMatrix(render::ColorMatrix{0, 0, 1, 0, 1, 0, 1, 0, 0});
// Draws the colors as they are again
window.setColorMatrix(std::nullopt);
```

A fragment shader applies the matrix when the game is shown in the window: the image of the logical resolution when there is one, otherwise an image of the size of the window that the game is drawn in, with the view of the window. The debug interface keeps its colors, and `mapPixelToWorld` gives the same points. `setColorMatrix` returns `false` when the graphics card has no shaders; the colors are then drawn as they are.

### Game area

`setGameArea` shows the game in a part of the window instead of the whole window, like the preview of the [editor layout](editor.md#layouts):

```cpp
// The game in the top half of an 800x600 window
window.setGameArea(Rectf{0.0F, 0.0F, 800.0F, 300.0F});
// The whole window again
window.setGameArea(std::nullopt);
```

The game keeps its proportions in the area, centered between black bars: here the world of 800x600 is shrunk to 400x300, from `(200, 0)`. At a logical resolution, `getPixelPerfectArea()` scales the image in the area instead of the window. `mapPixelToWorld` and `mapWorldToPixel` follow the area, and the debug interface keeps the whole window.

## The graphics library stays inside

The window is drawn by SFML, and the header of `Window` keeps it hidden. It declares the SFML type by name only and keeps the real window behind a pointer:

```cpp
namespace sf {
class RenderWindow;
}

class Window
{
    // ...
private:
    std::unique_ptr<sf::RenderWindow> _window;
};
```

A game that includes `younity/render/Window.hpp` compiles with the engine headers alone. The same rule applies to the rest of the module: the engine types are the public face, and the library does the work behind them.

## Textures

A `Texture` is an image loaded into the graphics card. `Texture::loadFromFile` has the signature of an `assets::Loader`, so the textures go through the resource manager and a file used by a hundred missiles is read once:

```cpp
assets::ResourceManager resources("assets");

resources.setLoader<render::Texture>(render::Texture::loadFromFile);

auto ship = resources.load<render::Texture>("sprites/ship.png");
```

A missing file gives `AssetError::NotFound`, and a file the graphics library cannot read gives `AssetError::LoadFailed`.

## Sprites

`Sprite` is the component that gives an entity its image. The entity also needs a `Transform`, which places, rotates and scales it.

```cpp
const ecs::Entity player = registry.createEntity();

registry.addComponent(player, ecs::Transform{.position = {100.0F, 300.0F}});
registry.addComponent(player, render::Sprite{.texture = *ship, .layer = 0});
```

| Member        | What it is                                                                |
|---------------|---------------------------------------------------------------------------|
| `texture`     | The image, shared with the resource manager. Nothing is drawn without one |
| `textureRect` | The part of the texture to draw, in pixels, like one image of a sheet. The whole texture if empty |
| `layer`       | The drawing order: lower layers first, so higher ones are on top          |
| `origin`      | The point of the image placed at the position, and the center of rotation |
| `visible`     | `false` hides the sprite without removing it, to make it blink            |
| `color`       | A `Color` that multiplies the colors of the texture, opaque white by default so the image is drawn as it is. A lower opacity fades it: `{255, 255, 255, 128}` is half see-through |

The sprite holds its texture through a shared pointer. The texture stays valid while one sprite uses it, even after the cache is emptied.

## Layers

The layer is a number, chosen by the game. A shoot'em up can put the background at `-1`, the ships at `0` and the interface at `10`. Inside a layer, the sprites are drawn by entity id.

```cpp
void onRender(float /* alpha */) override
{
    window.clear();
    // The first scene becomes active at its first step, which a very short first frame may not have
    if (scenes.hasActiveScene()) {
        render::drawEntities(scenes.getActiveScene().getRegistry(), window);
    }
    window.display();
}
```

`drawEntities` draws every visible sprite and text of the registry, from the lowest layer to the highest. Texts share the layers of the sprites, see [Texts](#texts). `getDrawOrder` gives that order without drawing, which the tests use.

Drawing is not a system of the registry. The systems run at every fixed step, zero to several times per frame, while drawing happens once per frame. `drawEntities` therefore asks the registry for its entities with `getEntitiesWith`, in `onRender`.

## Texts

A `Font` is a font file, like a TTF. Like the textures, it goes through the resource manager:

```cpp
resources.setLoader<render::Font>(render::Font::loadFromFile);

auto font = resources.load<render::Font>("fonts/arcade.ttf");
```

SFML reads the whole file when the font opens, so it can move or disappear afterwards. Keep the fonts and the textures in the game objects, like a scene or the application, never in a `static` or global variable: one destroyed after the end of `main` can crash the program on exit, once the graphics library has shut down.

`Text` is the component that shows a text, placed by the `Transform` of its entity, like a sprite:

```cpp
registry.addComponent(score, ecs::Transform{.position = {20.0F, 20.0F}});
registry.addComponent(score, render::Text{.font = *font, .string = "Score: 0", .characterSize = 24, .layer = 10});
```

| Member          | What it is                                                              |
|-----------------|-------------------------------------------------------------------------|
| `font`          | The font, shared with the resource manager. Nothing is drawn without one |
| `string`        | The text, in UTF-8: accents are shown, like `"Partie terminée"`         |
| `characterSize` | The height of the characters, in pixels, 30 by default                  |
| `color`         | A `Color`, white by default: `{255, 0, 0}` is red, a fourth number sets the opacity |
| `layer`         | The drawing order, shared with the sprites                              |
| `origin`        | The point of the text placed at the position, and the center of rotation |
| `visible`       | `false` hides the text without removing it                              |

Texts and sprites are drawn in the same pass, so they share the layers. A score on layer `10` is drawn over the ships of layer `0`. An entity can have both, like a button: in the same layer, its sprite is drawn first and its text over it.

`getTextBounds` measures a text, to center it for example:

```cpp
render::Text title{.font = *font, .string = "Game over", .characterSize = 48};
const Rectf bounds = render::getTextBounds(title);

title.origin = {bounds.left + bounds.width / 2.0F, bounds.top + bounds.height / 2.0F};
```

The title is then centered on the position of its `Transform`. Measuring renders the characters of the font, so it needs a graphics context. The graphics library creates one by itself, but only on a machine with a display: a dedicated server without one cannot measure texts.

## Entity at a point

`getEntityAt` gives the entity drawn at a point of the world, like the one under the mouse cursor:

```cpp
const Vector2f cursor = window.mapPixelToWorld({mouseX, mouseY});
const std::optional<ecs::Entity> clicked = render::getEntityAt(registry, cursor);
```

| Function         | Gives                                                                                          |
|------------------|------------------------------------------------------------------------------------------------|
| `getEntityAt`    | The entity of the last item of `getDrawOrder` whose rectangle holds the point, the one on top  |
| `getDrawCorners` | The four corners of the rectangle drawn for an item, after its origin, rotation, scale and position |

The rectangle of a sprite is its `textureRect`, or the current frame of its animation, or its whole texture. The rectangle of a text comes from `getTextBounds`, so it needs a graphics context. A hidden item, a sprite without texture and a text without font are at no point.

## Animations

A spritesheet holds the frames of an animation side by side, all of the same size. An `AnimationClip` lists the parts of the texture to show one after the other, how long each one is shown, and whether it starts again after the last one.

`AnimationClip::fromGrid` reads the frames of a sheet laid out as a grid. The frames are numbered from 0, left to right then top to bottom:

```cpp
// The 4 frames of the first row of a sheet of 32 by 32 frames, 4 per row, 10 per second
const render::AnimationClip flight = render::AnimationClip::fromGrid(
    {.frameSize = {32, 32}, .columns = 4, .first = 0, .count = 4}, 0.1F);

// The 6 frames of the second and third rows, played once
const render::AnimationClip explosion = render::AnimationClip::fromGrid(
    {.frameSize = {32, 32}, .columns = 4, .first = 4, .count = 6}, 0.05F, false);
```

The entity that plays it gets an `Animation` component next to its `Sprite`, and the scene adds the `AnimationSystem`:

```cpp
registry.addSystem<render::AnimationSystem, render::Animation>(getEventBus());

registry.addComponent(ship, render::Animation{.clip = flight});
```

The system moves the animations forward at each fixed step, so they run at the same speed at any frame rate. When drawing, the current frame of the `Animation` replaces the `textureRect` of the `Sprite`. A new entity therefore shows its first frame at once, even before the system has run.

`animation.play(explosion)` switches to another clip and starts it from its first frame. It restarts even when given the clip already playing, so the game calls it when the animation changes, not at every step.

A clip that does not loop stays on its last frame once it has shown it. The system then publishes an `AnimationFinishedEvent` once, and the game decides what it means:

```cpp
getEventBus().subscribe<render::AnimationFinishedEvent>([this](const render::AnimationFinishedEvent &event) {
    getRegistry().destroyEntity(event.entity);    // the explosion is over
});
```

The frames, the duration and the loop of the clip are registered fields of the `Animation` component: a scene file saves them, and the inspector edits them with the other fields.

| Type                     | Role                                                              |
|--------------------------|-------------------------------------------------------------------|
| `AnimationClip`          | The frames of an animation, their duration, and whether it loops  |
| `GridLayout`             | Where the frames are in a sheet laid out as a grid                |
| `Animation`              | The component: the clip an entity plays and its current frame     |
| `AnimationSystem`        | Moves the animations forward at each step                         |
| `AnimationFinishedEvent` | Published when a clip that does not loop has ended                |

## Particles

A `ParticleEmitter` spawns many short-lived sprites, the particles, at the `Transform` of its entity: explosions, engine trails, sparks. The scene adds the `ParticleSystem`:

```cpp
registry.addSystem<render::ParticleSystem, render::ParticleEmitter, ecs::Transform>();

// A trail going left behind the ship, 30 particles per second
render::ParticleEmitter trail{.rate = 30.0F, .lifetime = 0.4F, .speed = 80.0F, .direction = 180.0F, .spread = 20.0F};

trail.sprite.texture = *resources.load<render::Texture>("sprites/spark.png");
registry.addComponent(ship, trail);

// An explosion: 40 particles at once in every direction, then nothing
registry.addComponent(wreck, render::ParticleEmitter{.burst = 40, .emitting = false, .speed = 120.0F});
```

| Field            | What it sets                                                                 |
|------------------|------------------------------------------------------------------------------|
| `sprite`         | The sprite of each particle: its texture, part of the texture and layer       |
| `rate`           | Particles per second while `emitting` is true                                |
| `burst`          | Particles spawned at once at the next step, then set back to 0               |
| `emitting`       | `false` stops the flow; the burst and the particles already spawned go on    |
| `lifetime`       | Seconds a particle lives                                                     |
| `speed`, `speedVariation` | Each particle gets a speed between `speed - speedVariation` and `speed + speedVariation`, in pixels per second |
| `direction`      | Degrees, clockwise from the right like the rotation of a `Transform`: 90 goes down |
| `spread`         | The angle of the cone around `direction`: 0 is a straight line, 360 every direction |

At each fixed step, the system moves each particle by its velocity and destroys the ones whose life ended, then spawns the new particles of each emitter. A rate that does not fill a whole particle in one step, like 3 per second at 60 steps per second, keeps the part left for the next steps. A particle spawned in a step moves from the next one. An emitter spawns `MAX_PARTICLES_PER_STEP` particles at most in one step, 1000, and a negative rate spawns nothing, so a wrong value in a scene file cannot fill the memory.

A particle is an entity with a `Transform`, a copy of the `sprite` and a `Particle` holding its velocity and the time it has left, so `drawEntities` draws it like any sprite, sorted by its layer. Destroying the emitter leaves its particles to live their life. The directions and the speeds are random: `ParticleSystem(seed)` takes the seed of the random generator, fixed in the tests to get the same particles at each run.

## Debug interface

The window runs a [Dear ImGui](https://github.com/ocornut/imgui) interface over the game, through ImGui-SFML. `clear()` opens its frame, `display()` draws it on top of the entities, and `pollEvents()` gives it the mouse and the keyboard.

`drawDebugWindow` adds the debug window of the engine, called between `clear()` and `display()`:

```cpp
void onRender(float /* alpha */) override
{
    window.clear();
    if (scenes.hasActiveScene()) {
        render::drawEntities(scenes.getActiveScene().getRegistry(), window);
    }
    render::drawDebugWindow();
    window.display();
}
```

| Shown          | Value                                       |
|----------------|---------------------------------------------|
| Version        | The version of the engine, like `{{ younity_version }}` |
| FPS            | Frames per second, averaged over 60 frames  |
| ms per frame   | The time of one frame, from the same average |

The window can be moved and folded with the mouse. Its position is kept in memory for the run, so the game folder stays clean. `drawDebugWindow(area)` fixes it in an area of the window instead: the mouse can no longer move, resize or fold it. `beginDebugWindow(title, area)` begins any window of the debug interface the same way, like `ImGui::Begin()`; the [layouts of the editor](editor.md#layouts) place their windows with it. The console, the [metrics overlay](editor.md#metrics-overlay) and the editor are built on the same interface.

## Component registry

`render::registerComponents` registers `Sprite`, `Text`, `Animation`, `ParticleEmitter` and `Particle` in the [component registry](ecs.md#component-registry), with their data fields. It takes the resource manager of the game: the `texture` of a `Sprite` and the `font` of a `Text` are fields holding their path in the asset folder, so a scene file or the editor reads and writes them like the other fields.

```cpp
render::registerComponents(components, resources);
components.get("Sprite").setField(registry, ship, "texture", std::string("sprites/ship.png"));
```

Reading the field gives the path the resource was loaded from, or an empty path for a sprite without a texture or with one that did not come from the resource manager. Writing a path loads it, an empty path removes the texture, and a path that cannot be loaded throws `std::invalid_argument` and keeps the old texture. The resource manager must outlive the component registry.
