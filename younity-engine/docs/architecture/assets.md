# Assets

The `assets` module loads the resources of the game and keeps them, so a file used by a hundred missiles is read once. It knows how to find, cache and share a resource; reading a texture or a sound is the job of a loader given by the module that draws or plays it.

The headers are in `modules/assets/include/younity/assets/`, in the namespace `younity::assets`. The module is built on `core`.

## Loaders

A loader is a function that reads one resource from its file. It returns the resource, or the reason it could not:

```cpp
assets::Loader<Texture> loadTexture = [](const std::filesystem::path &path) -> std::expected<Texture, assets::AssetError> {
    Texture texture;

    if (!texture.loadFromFile(path)) {
        return std::unexpected(assets::AssetError::LoadFailed);
    }
    return texture;
};
```

A loader can load other resources, like a sheet of sprites that loads its picture. It must not call `setLoader` or `clear` for its own type, which would destroy it while it runs.

The module never opens a file itself. It stays free of the graphics and sound libraries, and a game, a dedicated server or a unit test gives it the loaders it needs.

## The resource manager

`ResourceManager` holds one cache per type of resource, all reading from the same asset folder. The game gives it a loader per type, then asks for resources by type and path:

```cpp
assets::ResourceManager resources("assets");

resources.setLoader<Texture>(loadTexture);
resources.setLoader<SoundBuffer>(loadSound);

auto ship = resources.load<Texture>("sprites/ship.png");

if (!ship) {
    // assets::toString(ship.error()) says why, e.g. "not found"
}
```

The first `load` of a file calls the loader, the next ones give the same resource from the cache. A failure is not kept: once the file is fixed, the next `load` reads it.

| Function            | What it does                                                   |
|---------------------|----------------------------------------------------------------|
| `setLoader<T>(f)`   | Sets how a type is read, and drops the resources of the old loader |
| `load<T>(path)`     | Gives the resource of a file, read the first time only         |
| `contains<T>(path)` | Tells if the resource of a file is in the cache                |
| `findPath(resource)` | Gives the file a resource of the cache was loaded from        |
| `unload<T>(path)`   | Removes the resource of a file from the cache                  |
| `clear()`           | Empties every cache and keeps the loaders                      |
| `getRoot()`         | Gives the asset folder, the one the paths are relative to      |

`ResourceCache<T>` is the cache of one type, with the same functions. The manager is built on it, and a module that handles a single type can use it directly.

## Shared resources

`load` gives a `std::shared_ptr<const T>`. The cache and every part of the game that uses the resource share it, and it is destroyed when the last of them lets it go.

A sprite that holds its texture keeps it valid, even after `unload` or `clear`. Changing scene can empty the cache without checking what the entities of the old scene still hold. The resource is read-only, so one entity cannot change the texture of all the others.

## Paths

A path is relative to the asset folder. The manager puts every path in its normal form before using it, so `sprites/ship.png`, `./sprites/ship.png` and `sprites/enemies/../ship.png` are one file and one entry of the cache.

A path that is empty, absolute, a folder, or that goes out of the asset folder, like `../save.dat`, is refused with `AssetError::InvalidPath` and the loader is not called. The check is lexical: it reads the name, not the file, so a symbolic link placed inside the asset folder is followed by the loader. The [security page](../study/security.md#asset-paths-stay-inside-the-asset-folder) says what this does and does not cover.

## Errors

| Error         | When                                               |
|---------------|----------------------------------------------------|
| `NotFound`    | The loader found no file at this path              |
| `LoadFailed`  | The file exists but the loader could not read it   |
| `InvalidPath` | The path is refused, see above                     |
| `NoLoader`    | No loader was given for this type of resource      |

A missing file is data from outside, so it is returned as a value. An empty loader is a bug in the code, and `setLoader` throws `std::invalid_argument`.

Every load is logged by the `assets` logger: an information when a file is read, a warning with the reason when it fails, and a debug message when the resource comes from the cache.

## Main types

| Type               | Role                                                    |
|--------------------|---------------------------------------------------------|
| `ResourceManager`  | One cache per type of resource, from one asset folder   |
| `ResourceCache<T>` | Loads and keeps the resources of one type               |
| `Loader<T>`        | The function that reads one resource from its file      |
| `AssetError`       | Why a resource could not be loaded                      |
