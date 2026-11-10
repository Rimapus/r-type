# Audio

The `audio` module plays the sound of the game: the short sounds, like a shot or an explosion, and the music.

The headers are in `modules/audio/include/younity/audio/`, in the namespace `younity::audio`. The module is built on `core` and `assets`, and plays through SFML, which stays hidden behind the engine types like in [Render](render.md#the-graphics-library-stays-inside).

## Short sounds

A `SoundBuffer` is a short sound read into memory. `SoundBuffer::loadFromFile` has the signature of an `assets::Loader`, so the sounds go through the resource manager and a sound played a hundred times is read once:

```cpp
assets::ResourceManager resources("assets");

resources.setLoader<audio::SoundBuffer>(audio::SoundBuffer::loadFromFile);

auto shot = resources.load<audio::SoundBuffer>("sounds/shot.wav");
```

A missing file gives `AssetError::NotFound`, and a file the audio library cannot read gives `AssetError::LoadFailed`. The library reads WAV, OGG, FLAC and MP3 files.

## Playing them

`SoundPlayer` plays the short sounds. The game asks for a sound and forgets it:

```cpp
audio::SoundPlayer sounds;

sounds.play(*shot);
sounds.play(*explosion, 0.5F);
```

The player has a fixed number of voices, 32 by default (`DEFAULT_VOICES`), and each voice plays one sound at a time. A new sound takes a silent voice. When all of them are playing, it takes the voice of the sound that started first: in a busy shoot'em up, the newest sound is the one that matches what happens on screen.

A voice keeps a shared pointer to its buffer until it plays another sound. The buffer stays valid while it may be heard, even after the cache of the resource manager is emptied.

| Function            | What it does                                                         |
|---------------------|----------------------------------------------------------------------|
| `play(buffer, volume)` | Plays a sound once; `false` if there is no sound to play          |
| `stopAll()`         | Stops every sound playing                                            |
| `setVolume(volume)` | Sets the volume of all the sounds, the ones playing included         |
| `getVolume()`       | The volume of all the sounds                                         |
| `getVoiceCount()`   | The number of sounds that can play at the same time                  |
| `getPlayingCount()` | The number of sounds playing now                                     |

## Music

A `Music` is a long sound, read from its file while it plays instead of all at once. The file must stay in place while the music exists.

```cpp
auto music = audio::Music::openFromFile("assets/music/level1.ogg");

if (music) {
    music->setLooping(true);
    music->play();
}
```

The game owns its music directly, in a scene or the application. It does not go through the resource manager: the manager shares resources that cannot be changed, and a music changes when it plays.

| Function             | What it does                                                    |
|----------------------|-----------------------------------------------------------------|
| `play()`             | Starts the music, from the beginning or from where it was paused |
| `pause()`            | Pauses it                                                       |
| `stop()`             | Stops it, the next `play()` starts from the beginning           |
| `isPlaying()`        | Tells if it is playing                                          |
| `setLooping(looping)`| Starts it again when it is over, off by default                 |
| `isLooping()`        | Tells if it starts again when it is over                        |
| `setVolume(volume)`  | Sets its volume                                                 |
| `getVolume()`        | Its volume                                                      |

## Volumes

A volume goes from `0` for silence to `1` for full volume, and a value out of that range is brought back into it. The volume given to `play` is multiplied by the volume of the player, so a settings menu changes all the sounds at once with `setVolume`.

## Without audio output

A machine without audio output, like a CI container, still loads the sounds and opens the music. Nothing plays: `getPlayingCount()` stays `0` and `isPlaying()` stays `false`. The volumes and the looping of a music are kept by the engine, so they read back the same with or without an output. The tests that need to hear a sound skip themselves on such a machine, see `tests/audio/`.

A sound needs no step of the [frame](frame.md): a system calls `play` during a fixed step, and the audio library mixes the sounds on its own thread. The engine types are not thread-safe: call them from the thread of the game loop only.
